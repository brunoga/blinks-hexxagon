#include "game_state_end.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace end {

static bool sent_flash_;
static byte winner_player_;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)specific_state;

  if (state_changed) {
    byte max_count = 0;
    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      byte player_count = GetBlinkCount()[i];
      if (player_count > max_count) {
        max_count = player_count;
        winner_player_ = i;
      } else if (player_count == max_count) {
        winner_player_ = 0;
      }
    }

    sent_flash_ = game::state::FromNetwork();
  }

  if (util::NoSleepButtonSingleClicked()) {
    // Ok to ignore result.
    game::message::SendFlash();

    *state = GAME_STATE_IDLE;

    return;
  }

  if (!sent_flash_ && !game::message::SendFlash()) return;

  sent_flash_ = true;

  // Switch to the winner player.
  blink::state::SetPlayer(winner_player_);
}

}  // namespace end

}  // namespace state

}  // namespace game