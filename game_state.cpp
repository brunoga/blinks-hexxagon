
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

void Set(byte state, bool from_network) {
  state_.previous = state_.current;
  state_.current = state;

  state_.from_network = from_network;
}

byte Get() { return state_.current; }

void SetSpecific(byte specific_state, bool from_network) {
  state_.previous_specific = state_.current_specific;
  state_.current_specific = specific_state;

  state_.from_network = from_network;
}

byte GetSpecific() { return state_.current_specific; }

void __attribute__((noinline)) SetPlayer(byte player) {
  state_.player = player;
}

byte __attribute__((noinline)) GetPlayer() { return state_.player; }

void NextPlayer() {
  byte current_player = GetPlayer();

  byte next_player = game::player::GetNext(current_player);
  while (((game::map::GetStats().player_blink_count[next_player] == 0) ||
          (next_player == 0)) &&
         (next_player != current_player)) {
    next_player = game::player::GetNext(next_player);
  }

  SetPlayer(next_player);
}

void Reset() {
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

  game::message::GameStateChangeData data;

  data.state = state_.current;
  data.specific_state = state_.current_specific;
  data.next_player = state_.player - 1;

  if (!game::message::SendGameStateChange(data.value)) {
    return false;
  }

  return true;
}

bool FromNetwork() { return state_.from_network; }

}  // namespace state

}  // namespace game
