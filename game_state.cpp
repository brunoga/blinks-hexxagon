
#include "game_state.h"

#include <string.h>

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"

namespace game {

struct State {
  byte current;
  byte previous;
  byte player;
  byte winner_player;
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

  // TODO(bga): Note the loop below might stall if this function is ever called
  // when there are no players that can move at all. This is never the case
  // currently and as not doing a specific check for this here saves 20 bytes,
  // we will leave it at that.

  // Start with the current player.
  byte next_player = game::player::GetNext(current_player);

  for (;;) {
    const auto& player_stats = game::map::GetStatistics().player[next_player];
    if ((player_stats.blink_count != 0) && (next_player != 0) &&
        player_stats.can_move) {
      // This player can move, return it.
      break;
    } else {
      // This player can not move.
      if ((player_stats.blink_count != 0) && (next_player != 0)) {
        // And it was because there was no place for it to move. Send flash.
        blink::state::StartColorOverride();
      }

      // Try next player.
      next_player = game::player::GetNext(next_player);
    }
  }

  SetPlayer(next_player);
}

void SetWinnerPlayer(byte winner_player) {
  state_.winner_player = winner_player;
}

byte GetWinnerPlayer() { return state_.winner_player; }

byte GetData() { return Data{0, state_.current, state_.player}.as_byte; }

void Reset() { memset(&state_, 0, sizeof(state_)); }

bool Changed() { return state_.current != state_.previous; }

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
