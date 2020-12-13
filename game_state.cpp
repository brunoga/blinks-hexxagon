
#include "game_state.h"

#include "game_map.h"
#include "game_message.h"
#include "game_player.h"

namespace game {

struct State {
  byte current;
  byte previous;
  byte current_specific;
  byte previous_specific;
  byte player;
  bool from_network;
};
static State state_;

namespace state {

void __attribute__((noinline)) Set(byte state, bool from_network) {
  state_.previous = state_.current;
  state_.current = state;

  state_.from_network = from_network;
}

byte __attribute__((noinline)) Get() { return state_.current; }

void __attribute__((noinline))
SetSpecific(byte specific_state, bool from_network) {
  state_.previous_specific = state_.current_specific;
  state_.current_specific = specific_state;

  state_.from_network = from_network;
}

byte GetSpecific() { return state_.current_specific; }

void SetPlayer(byte player) { state_.player = player; }

byte GetPlayer() { return state_.player; }

void NextPlayer() {
  byte current_player = GetPlayer();

  byte next_player = game::player::GetNext(current_player);
  while (((game::map::GetStatistics().player[next_player].blink_count == 0) ||
          (next_player == 0) ||
          !game::map::GetStatistics().player[next_player].can_move) &&
         (next_player != current_player)) {
    next_player = game::player::GetNext(next_player);
  }

  SetPlayer(next_player);
}

byte GetData() {
  return Data{state_.current, state_.current_specific,
              (byte)(state_.player - 1)}
      .as_byte;
}

void __attribute__((noinline)) Reset() {
  state_.current = GAME_STATE_IDLE;
  state_.previous = GAME_STATE_IDLE;
  state_.current_specific = 0;
  state_.previous_specific = 0;
  state_.player = 0;
  state_.from_network = false;
}

bool Changed(bool include_specific) {
  return include_specific
             ? state_.current != state_.previous ||
                   state_.current_specific != state_.previous_specific
             : state_.current != state_.previous;
}

bool Propagate() {
  if (!Changed() || state_.from_network) return true;

  if (!game::message::SendGameStateChange()) {
    return false;
  }

  return true;
}

bool FromNetwork() { return state_.from_network; }

}  // namespace state

}  // namespace game
