#include "game_state_end.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace end {

static bool sent_flash_;
static byte winner_player_;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)state;
  (void)specific_state;

  if (!blink::state::GetTarget()) return;

  if (state_changed) {
    byte max_count = 0;
    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      byte player_count = GetBlinkCount(i);
      if (player_count > max_count) {
        max_count = player_count;
        winner_player_ = i;
      } else if (player_count == max_count) {
        winner_player_ = 0;
      }
    }

    sent_flash_ = false;
  }

  if (!sent_flash_ && !game::message::SendFlash()) return;

  sent_flash_ = true;

  if (!game::message::SendReportWinner(winner_player_)) return;

  // Make sure we will not just continue sending messages.
  blink::state::SetTarget(false);
}  // namespace end

}  // namespace end

}  // namespace state

}  // namespace game