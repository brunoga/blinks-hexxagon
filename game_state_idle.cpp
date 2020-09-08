#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "manager.h"

namespace game {

namespace state {

namespace idle {

// Idle state handler. This just waits for a click to move to the setup state.
void Handler(bool state_changed, byte* state) {
  if (state_changed) {
    // We just changed to this state. Reset global (game) and local (Blink)
    // states.
    game::state::Reset();
    blink::state::Reset();
  }

  if (!buttonSingleClicked() || hasWoken()) return;

  // Switch to setup state.
  *state = GAME_STATE_SETUP;
}

}  // namespace idle

}  // namespace state

}  // namespace game
