#include "game_state_idle_render.h"

#include "game_player.h"
#include "render_animation.h"
#include "render_config.h"

namespace game {

namespace state {

namespace idle {

void Render() {
  render::animation::Pulse(render::animation::Empty,
                           RENDER_CONFIG_IDLE_STATE_PULSE_START_DIM,
                           RENDER_CONFIG_IDLE_STATE_PULSE_SLOWDOWN);
}

}  // namespace idle

}  // namespace state

}  // namespace game