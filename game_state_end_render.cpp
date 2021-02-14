#include "game_state_end_render.h"

#include <hexxagon_config.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_player.h"

namespace game {

namespace state {

namespace end {

void Render() {
  blink::state::render::Pulse(HEXXAGON_RENDER_END_STATE_PULSE_START_DIM,
                              HEXXAGON_RENDER_END_STATE_PULSE_SLOWDOWN);
}

}  // namespace end

}  // namespace state

}  // namespace game