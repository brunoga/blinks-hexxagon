#include "game_state_idle_render.h"

#include "game_player.h"
#include "render_animation.h"

namespace game {

namespace state {

namespace idle {

void Render() {
  render::animation::Pulse(game::player::GetColor(0), /*start=*/128,
                           /*speed=*/10);
}

}  // namespace idle

}  // namespace state

}  // namespace game