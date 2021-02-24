#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace idle {

void Handler(byte* state) {
  // To simplify logic we juts keep reseting the global (game) and local (Blink)
  // states every loop iteration here. This is kinda harmless.
  game::state::Reset();
  blink::state::Reset();

  if (!buttonDoubleClicked()) return;

  *state = GAME_STATE_SETUP;
}

}  // namespace idle

}  // namespace state

}  // namespace game
