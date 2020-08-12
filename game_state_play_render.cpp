#include "game_state_play_render.h"

#include "blink_state.h"
#include "debug.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace play {

static Timer selected_timer_;

void renderSelected(const Color& base_color) {
  if (selected_timer_.isExpired()) {
    selected_timer_.set((5 * 100) + 99);
  }

  byte f = selected_timer_.getRemaining() / 100;

  setColor(base_color);
  setColorOnFace(WHITE, f);
}

void Render() {
  byte current_player = blink::state::GetPlayer();
  Color current_player_color = game::player::GetColor(current_player);

  switch (current_player) {
    case 0:
      if (blink::state::GetTarget()) {
        renderSelected(current_player_color);
        return;
      }

      if (blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_NONE) {
        current_player_color = dim(current_player_color, 191);
      } else {
        current_player_color = dim(current_player_color, 127);
      }
      break;
    default: {
      if (game::state::GetPlayer() == blink::state::GetPlayer()) {
        if (!blink::state::GetOrigin()) {
          current_player_color = dim(current_player_color, 191);
        } else {
          renderSelected(current_player_color);
          return;
        }
      } else {
        current_player_color = dim(current_player_color, 127);
      }
      break;
    }
  }

  setColor(current_player_color);
}

}  // namespace play

}  // namespace state

}  // namespace game