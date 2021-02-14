#include "game_state_end_render.h"

#include "blink_state.h"
#include "game_player.h"
#include "render_animation.h"
#include "render_config.h"

namespace game {

namespace state {

namespace end {

void Render() {
  if (blink::state::GetPlayer() == GAME_PLAYER_NO_PLAYER) {
    render::animation::Pulse(render::animation::Empty,
                             RENDER_CONFIG_END_STATE_PULSE_START_DIM,
                             RENDER_CONFIG_END_STATE_PULSE_SLOWDOWN);
  } else {
    render::animation::Pulse(game::player::GetColor(blink::state::GetPlayer()),
                             RENDER_CONFIG_END_STATE_PULSE_START_DIM,
                             RENDER_CONFIG_END_STATE_PULSE_SLOWDOWN);
  }
}

}  // namespace end

}  // namespace state

}  // namespace game