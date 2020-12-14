#include "game_state_play_render.h"

#include "blink_state.h"
#include "game_map.h"
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

  byte game_player = game::state::GetPlayer();
  byte specific_state = game::state::GetSpecific();

  // Set initial color for this Blink. This is required as some of the render
  // animations only render an overlay.
  setColor(player_color);

  if (player == 0) {
    if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
      setColor(dim(player_color, RENDER_CONFIG_PLAY_STATE_COLOR_DIM));
    }
  } else {
    if (player == game_player) {
      if (blink::state::GetOrigin() &&
          specific_state < GAME_STATE_PLAY_TARGET_SELECTED) {
        // We are the Origin. Render the spinning animation.
        render::animation::Spinner(RENDER_CONFIG_PLAY_STATE_SPINNER_COLOR,
                                   RENDER_CONFIG_PLAY_STATE_SPINNER_SLOWDOWN);
      } else if ((specific_state < GAME_STATE_PLAY_MOVE_CONFIRMED) &&
                 game::map::GetStatistics().local_blink_empty_space_in_range) {
        // This Blink belongs to the current player and did not match any of the
        // above conditions. Render a pulsing animation if we are not confirming
        // the move yet.
        render::animation::Pulse(player_color,
                                 RENDER_CONFIG_PLAY_STATE_PULSE_START_DIM,
                                 RENDER_CONFIG_PLAY_STATE_PULSE_SLOWDOWN);
      }
    } else {
      if ((specific_state == GAME_STATE_PLAY_MOVE_CONFIRMED) &&
          position::Distance(game::map::GetMoveTarget()) == 1) {
        // Render takeover (explosion) animation.
        if (render::animation::Explosion(player_color)) {
          blink::state::SetPlayer(game_player);
        }
      }
    }
  }
}

}  // namespace play

}  // namespace state

}  // namespace game