#include "game_state_setup.h"

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_player.h"
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

    byte result = game::state::UpdateBoardState();
    if (result == GAME_STATE_UPDATE_BOARD_STATE_UPDATING) {
      return;
    }

    checking_board_ = false;

    if (result == GAME_STATE_UPDATE_BOARD_STATE_ERROR) {
      // Tell all Blinks to flash. Should be ok to ignore the return value here.
      // TODO(bga): Make sure it is or fix this.
      game::message::SendFlash();

      return;
    }

    // Swithc to first available player.
    game::state::NextPlayer();

    *state = GAME_STATE_PLAY;
    *specific_state = 0;

    return;
  }

  if (!buttonSingleClicked() || hasWoken()) return;

  blink::state::SetPlayer(game::player::GetNext(blink::state::GetPlayer()));
}

}  // namespace setup

}  // namespace state

}  // namespace game
