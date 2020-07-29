#include "game_state_setup.h"

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"

namespace game {

namespace state {

namespace setup {

bool checking_board_ = false;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    blink::state::Reset();
  }

  if (buttonDoubleClicked() || checking_board_) {
    checking_board_ = true;

    broadcast::message::Message reply;
    if (!game::message::SendCheckBoard(reply)) {
      return;
    }

    checking_board_ = false;

    const byte* payload = broadcast::message::Payload(reply);
    byte empty_blinks = payload[0] - (payload[1] + payload[2]);
    if (payload[1] == 0 || payload[2] == 0 || empty_blinks == 0) {
      // We need at least one piece for player one and one piece for player two
      // and one empty piece.

      // TODO(bga): Add some visual feedback to indicate something is wrong.
      return;
    }

    *state = GAME_STATE_PLAY;
    *specific_state = 0;
  }

  if (buttonSingleClicked()) {
    // TODO(bga): This is so ugly. Just do a for loop. :)
    switch (blink::state::GetPlayer()) {
      case 0:
        blink::state::SetPlayer(1);
        break;
      default: {
        if (blink::state::GetPlayer() == 1) {
          blink::state::SetPlayer(2);
        } else {
          blink::state::SetPlayer(0);
        }
        break;
      }
    }
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game
