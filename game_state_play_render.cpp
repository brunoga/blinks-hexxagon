#include "game_state_play_render.h"

#include "blink_state.h"
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

  if ((blink::state::GetTarget() && blink::state::GetPlayer() == 0) ||
      blink::state::GetOrigin()) {
    render::animation::Spinner(player_color, WHITE, 1, 100);
    return;
  }

  if (game::state::GetPlayer() == player) {
    render::animation::Pulse(player_color, 64, 3);
    return;
  }

  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
    setColor(dim(player_color, 94));
    return;
  }

  setColor(player_color);
}

}  // namespace play

}  // namespace state

}  // namespace game