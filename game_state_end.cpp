#include "game_state_end.h"

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace end {

void Handler(byte* state) {
  byte max_count = 0;
  byte winner_player = 0;
  for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
    byte player_count = game::map::GetStatistics().player[i].blink_count;
    if (player_count > max_count) {
      max_count = player_count;
      winner_player = i;
    } else if (player_count == max_count) {
      winner_player = 0;
    }
  }

  // Switch to the winner player.
  blink::state::SetPlayer(winner_player);

  if (util::NoSleepButtonSingleClicked()) {
    // Ok to ignore result.
    game::message::SendFlash();

    *state = GAME_STATE_IDLE;

    return;
  }
}

}  // namespace end

}  // namespace state

}  // namespace game