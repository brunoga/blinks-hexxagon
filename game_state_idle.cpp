#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"

namespace game {

namespace state {

namespace idle {

void Handler(byte* state, bool button_double_clicked) {
  // To simplify logic we juts keep reseting the global (game) and local (Blink)
  // states every loop iteration here. This is kinda harmless.
  game::state::Reset();
  blink::state::Reset();

  if (!button_double_clicked) return;

  *state = GAME_STATE_SETUP_SELECT_PLAYERS;
}

}  // namespace idle

}  // namespace state

}  // namespace game
