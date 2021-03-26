#include "game_state_idle.h"

#include "blink_state.h"
#include "blink_state_face.h"
#include "blink_state_render.h"
#include "game_message.h"
#include "game_state.h"

namespace game {

namespace state {

namespace idle {

void Handler(byte* state, bool button_single_clicked) {
  // Reset all relevant state.
  game::state::Reset();
  blink::state::Reset();
  blink::state::face::handler::Reset();

  // Reset any possible pending datagram. This prevents the game from locking
  // up under some edge-case conditions.
  FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }

  if (!button_single_clicked) return;

  // Immediately switch to player 1.
  blink::state::SetPlayer(1);

  *state = GAME_STATE_SETUP_SELECT_PLAYERS;
}

}  // namespace idle

}  // namespace state

}  // namespace game
