#include "game_state_idle_render.h"

#include <hexxagon_config.h>

#include "blink_state_render.h"
#include "game_player.h"

namespace game {

namespace state {

namespace idle {

void Render() {
  blink::state::render::Pulse(blink::state::render::Empty,
                              HEXXAGON_RENDER_IDLE_STATE_PULSE_START_DIM,
                              HEXXAGON_RENDER_IDLE_STATE_PULSE_SLOWDOWN);
}

}  // namespace idle

}  // namespace state

}  // namespace game