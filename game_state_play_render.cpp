#include "game_state_play_render.h"

#include "blink_state.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_play.h"
#include "render_animation.h"
#include "render_config.h"

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
    render::animation::Spinner(RENDER_CONFIG_PLAY_STATE_SPINNER_COLOR,
                               RENDER_CONFIG_PLAY_STATE_SPINNER_FACES,
                               RENDER_CONFIG_PLAY_STATE_SPINNER_SLOWDOWN);
  } else if (game::state::GetPlayer() == player &&
             game::state::GetSpecific() < GAME_STATE_PLAY_CONFIRM_MOVE) {
    // This Blink belongs to the current player and did not match any of the
    // above conditions. Render a pulsing animation if we are not confirming the
    // move yet.
    render::animation::Pulse(player_color,
                             RENDER_CONFIG_PLAY_STATE_PULSE_START_DIM,
                             RENDER_CONFIG_PLAY_STATE_PULSE_SLOWDOWN);
  } else if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
    // We did not match anything above and we are not a target. Render a dimmed
    // version of the player color.
    setColor(dim(player_color, RENDER_CONFIG_PLAY_STATE_COLOR_DIM));
  }
}

}  // namespace play

}  // namespace state

}  // namespace game