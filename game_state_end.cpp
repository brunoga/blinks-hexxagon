#include "game_state_end.h"

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace end {

void Handler(byte* state, bool button_double_clicked) {
  byte max_count = 0;
  for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS; ++i) {
    byte player_count = game::map::GetStatistics().player[i].blink_count;
    if (player_count > max_count) {
      max_count = player_count;
      game::state::SetWinnerPlayer(i);
    } else if (player_count == max_count) {
      game::state::SetWinnerPlayer(0);
    }
  }

  if (!button_double_clicked) return;

  *state = GAME_STATE_IDLE;
  blink::state::face::handler::ResetGame();
}

}  // namespace end

}  // namespace state

}  // namespace game