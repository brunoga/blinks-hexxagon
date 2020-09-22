#include "game_state_play_render.h"

#include "blink_state.h"
#include "game_player.h"
#include "game_state.h"
#include "render_animation.h"

namespace game {

namespace state {

namespace play {

void Render() {
  // TODO(bga): Attempts to reduce code size here failed miserably. But my
  // intuition is that it can be done. Try again?

  byte player = blink::state::GetPlayer();
  Color player_color = game::player::GetColor(player);

  // Set initial color for this Blink. This is required as some of the render
  // animations only render an overlay.
  setColor(player_color);

  if (blink::state::GetAnimating()) {
    // Render predefined animation. Currently this is always lightning +
    // explosion.
    if (blink::state::RunAnimatingFunction()) {
      blink::state::SetAnimating(false);
    }
  } else if ((blink::state::GetTarget() && player == 0) ||
             blink::state::GetOrigin()) {
    // We are either the target Blink (before it is effectively taken over) or
    // the Origin. Render the spinning animation.
    render::animation::Spinner(WHITE, 3, 50);
  } else if (game::state::GetPlayer() == player) {
    // This Blink belongs to the current player and did not match any of the
    // above conditions. Render a pulsing animation.
    render::animation::Pulse(player_color, 64, 3);
  } else if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
    // We did n ot match anything above and we are not a target. Render a dimmed
    // version of the player color.
    setColor(dim(player_color, 94));
  }
}

}  // namespace play

}  // namespace state

}  // namespace game