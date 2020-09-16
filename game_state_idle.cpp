#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace idle {

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)specific_state;

  if (state_changed) {
    // We just changed to this state. Reset global (game) and local (Blink)
    // states.
    game::state::Reset();
    blink::state::Reset();

    *specific_state = 0;
  }

  if (!util::NoSleepButtonSingleClicked()) return;

  // Switch to setup state.
  *state = GAME_STATE_SETUP;
}

}  // namespace idle

}  // namespace state

}  // namespace game
