#include "game_state_idle.h"

#include "blink_state.h"
#include "blink_state_face.h"
#include "blink_state_render.h"
#include "game_message.h"
#include "game_state.h"

namespace game {

namespace state {

namespace idle {

void Handler(byte* state, bool button_double_clicked) {
  // TODO(bga): This check and the ResetPulseTimer() call bellow increases
  // storage usage by 30 bytes. If storage pressure increases again, remove this
  // from here (it is just for aesthetics).
  if (game::state::Changed()) {
    // Synchronize pulse as best as possible.
    blink::state::render::ResetPulseTimer();

    // Reset all relevant state.
    game::state::Reset();
    blink::state::Reset();
    blink::state::face::handler::Reset();

    // Reset any possible pending datagram. This prevents the game from locking
    // up under some edge-case conditions.
    FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }
  }

  if (!button_double_clicked) return;

  *state = GAME_STATE_SETUP_SELECT_PLAYERS;
}

}  // namespace idle

}  // namespace state

}  // namespace game
