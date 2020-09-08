#include "game_state_play_render.h"

#include "blink_state.h"
#include "debug.h"
#include "game_player.h"
#include "game_state.h"
#include "render_animation.h"

namespace game {

namespace state {

namespace play {

void Render() {
  byte player = blink::state::GetPlayer();
  Color player_color = game::player::GetColor(player);

  if (blink::state::GetExploding()) {
    if (render::animation::Explosion(player_color)) {
      blink::state::SetExploding(false);
    }
    return;
  }

  if (blink::state::GetTarget() || blink::state::GetOrigin()) {
    render::animation::WhiteSpinner(player_color);
    return;
  }

  if (game::state::GetPlayer() == player) {
    render::animation::Pulse(player_color, 128, 5);
    return;
  }

  setColor(dim(player_color, 94));
}

}  // namespace play

}  // namespace state

}  // namespace game