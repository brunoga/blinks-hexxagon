#include "game_state_play_render.h"

#include <hexxagon_config.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_map.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_play.h"

namespace game {

namespace state {

namespace play {

void Render() {
  // TODO(bga): Attempts to reduce code size here failed miserably. But my
  // intuition is that it can be done. Try again?

  byte player = blink::state::GetPlayer();
  Color player_color = game::player::GetColor(player);

  byte game_player = game::state::GetPlayer();
  byte state = game::state::Get();

  // Set initial color for this Blink. This is required as some of the render
  // animations only render an overlay.
  blink::state::render::Player(255);

  if ((blink::state::GetOrigin() &&
       (state < GAME_STATE_PLAY_TARGET_SELECTED)) ||
      (blink::state::GetTarget() &&
       (state == GAME_STATE_PLAY_MOVE_CONFIRMED))) {
    // We are the origin and and a target was not selected yet or we are a
    // target and the move is being confirmed. Render spinning animation.
    blink::state::render::Spinner(HEXXAGON_RENDER_PLAY_STATE_SPINNER_COLOR,
                                  HEXXAGON_RENDER_PLAY_STATE_SPINNER_SLOWDOWN);
  } else if (player == game_player) {
    // We are the current player.
    if ((state < GAME_STATE_PLAY_MOVE_CONFIRMED) &&
        game::map::GetStatistics().local_blink_empty_space_in_range) {
      // Move has not been confirmed yet and the Blink has a space in range to
      // move to (or is a possible target). Render pulse animation.
      blink::state::render::Pulse(HEXXAGON_RENDER_PLAY_STATE_PULSE_START_DIM,
                                  HEXXAGON_RENDER_PLAY_STATE_PULSE_SLOWDOWN);
      return;
    }
  } else if (player != 0) {
    // We are an enemy player Blink.
    if ((state == GAME_STATE_PLAY_MOVE_CONFIRMED) &&
        position::Distance(game::map::GetMoveTarget()) == 1) {
      // Move is being confirmed and we are neighbors of the target. Render
      // explosion animation.
      if (blink::state::render::Explosion(player_color)) {
        blink::state::SetPlayer(game_player);
      }

      return;
    }
  } else if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
    // We did not match anything above, which means we are a locked player
    // Blink, an unaffected enemy Blink or a non-target empty Blink.
    if (state == GAME_STATE_PLAY_SELECT_TARGET &&
        player == GAME_PLAYER_NO_PLAYER) {
      blink::state::render::Player(
          HEXXAGON_RENDER_PLAY_STATE_SELECT_TARGET_COLOR_DIM);
    } else {
      blink::state::render::Player(HEXXAGON_RENDER_PLAY_STATE_COLOR_DIM);
    }
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
