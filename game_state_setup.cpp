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
    // TODO(bga): Expand to 4 players.
    byte current_player = blink::state::GetPlayer();
    blink::state::SetPlayer(current_player < 2 ? current_player + 1 : 0);
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game
