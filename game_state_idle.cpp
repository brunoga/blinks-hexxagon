#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "manager.h"

namespace game {

namespace state {

namespace idle {

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    game::state::Reset();
    blink::state::Reset();
  }

  if (buttonSingleClicked()) {
    // Switch to setup state.
    *state = GAME_STATE_SETUP;
    *specific_state = 0;

    return;
  }
}

}  // namespace idle

}  // namespace state

}  // namespace game
