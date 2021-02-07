
#include "game_state.h"

#include "game_map.h"
#include "game_message.h"
#include "game_player.h"

namespace game {

struct State {
  byte current;
  byte previous;
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

byte Get() { return state_.current; }

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

byte GetData() { return Data{0, state_.current, state_.player}.as_byte; }

void __attribute__((noinline)) Reset() {
  state_.current = GAME_STATE_IDLE;
  state_.previous = GAME_STATE_IDLE;
  state_.player = 0;
  state_.from_network = false;
}

bool __attribute__((noinline)) Changed() {
  return state_.current != state_.previous;
}

bool Propagate() {
  if (!state_.from_network && Changed() &&
      !game::message::SendGameStateChange()) {
    return false;
  }

  return true;
}

bool FromNetwork() { return state_.from_network; }

}  // namespace state

}  // namespace game
