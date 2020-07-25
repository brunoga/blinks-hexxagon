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
  (void)state_changed;

  if (buttonDoubleClicked() || checking_board_) {
    checking_board_ = true;

    broadcast::message::Message reply;
    if (!game::message::SendCheckBoard(reply)) {
      *state = GAME_STATE_SETUP;
      *specific_state = 0;

      return;
    }

    checking_board_ = false;

    const byte* payload = broadcast::message::Payload(reply);
    if (payload[1] == 0 || payload[2] == 0) {
      // We need at least one piece for player one and one piece for player
      // two.

      // TODO(bga): Add some visual feedback to indicate something is wrong.
      *state = GAME_STATE_SETUP;
      *specific_state = 0;

      return;
    }

    *state = GAME_STATE_PLAY;
    *specific_state = 0;

    return;
  }

  if (buttonSingleClicked()) {
    switch (blink::state::GetType()) {
      case BLINK_STATE_TYPE_EMPTY:
        blink::state::SetType(BLINK_STATE_TYPE_PLAYER);
        blink::state::SetPlayer(1);
        break;
      case BLINK_STATE_TYPE_PLAYER: {
        if (blink::state::GetPlayer() == 1) {
          blink::state::SetPlayer(2);
        } else {
          blink::state::SetType(BLINK_STATE_TYPE_EMPTY);
          blink::state::SetPlayer(0);
        }
        break;
      }
    }
  }

  *state = GAME_STATE_SETUP;
  *specific_state = 0;
}

void HandleReceiveMessage(byte message_id, byte* payload) {
  (void)message_id;
  (void)payload;
}

}  // namespace setup

}  // namespace state

}  // namespace game
