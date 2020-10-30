#include "game_state_setup.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace setup {

static bool checking_board_ = false;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)state_changed;

  if (checking_board_) {
    // We need to either start validating the game state or we are already doing
    // it.
    byte result = game::state::UpdateBoardState();
    if (result == GAME_STATE_UPDATE_BOARD_STATE_UPDATING) {
      // Still checking. Try again next loop() iteration.
      return;
    }

    // We finished checking. mark it as so so we do not keep doing it over and
    // over.
    checking_board_ = false;

    if (result == GAME_STATE_UPDATE_BOARD_STATE_OK) {
      // Game state is good. Switch to first available player.
      game::state::NextPlayer();

      // And move to the play state.
      *state = GAME_STATE_PLAY;

      // Strictly speaking, we do not need this but if we do not do it, we
      // actually *INCREASE* the binary size. Simply removing the parameter
      // (thus also not doing this here) also increases it.
      //
      // TODO(bga): Understand why and how can we use it in our benefit. Note
      // that this same trick does not work in other state handlers.
      *specific_state = 0;
    }
  } else if (buttonDoubleClicked()) {
    // We seem to be done with setting up the game. We now need to validate if
    // the board state is actually valid.
    checking_board_ = true;
  } else if (util::NoSleepButtonSingleClicked()) {
    // Blink was clicked. Switch it to next player. Note that we will never
    // reach this point if we are validating the game state until we finish
    // doing it. This is intentional and prevents the local state changing
    // while we are validating the board. Clicking other Blinks could
    // potentially mess things up, but there is only so much we can do with
    // the resources available so we ignore it.
    blink::state::SetPlayer(game::player::GetNext(blink::state::GetPlayer()));
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game