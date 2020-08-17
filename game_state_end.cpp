#include "game_state_end.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace end {

static byte winner_player_;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (!blink::state::GetArbitrator()) return;

  if (state_changed) {
    byte max_count = 0;
    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      if (game::state::GetBlinkCount(i) > max_count) {
        max_count = game::state::GetBlinkCount(i);
      }
    }

    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      if (game::state::GetBlinkCount(i) == max_count) {
        if (winner_player_ != 0) {
          winner_player_ = 0;
          break;
        }

        winner_player_ = i;
      }
    }

    blink::state::SetPlayer(winner_player_);
  }

  if (!game::message::SendReportWinner(winner_player_)) return;

  blink::state::SetArbitrator(false);
}

}  // namespace end

}  // namespace state

}  // namespace game