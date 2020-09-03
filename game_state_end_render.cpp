#include "game_state_end_render.h"

#include "blink_state.h"
#include "game_player.h"
#include "render_animation.h"

namespace game {

namespace state {

namespace end {

void Render() {
  render::animation::Pulse(game::player::GetColor(blink::state::GetPlayer()),
                           128, 5);
}

}  // namespace end

}  // namespace state

}  // namespace game