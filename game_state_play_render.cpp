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
  byte player = blink::state::GetPlayer();
  Color player_color = game::player::GetColor(player);

  switch (player) {
    case 0:
      if (blink::state::GetTarget()) {
        renderSelected(player_color);
        return;
      }

      if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) {
        player_color = dim(player_color, 94);
      }
      break;
    default: {
      if (game::state::GetPlayer() == blink::state::GetPlayer()) {
        if (blink::state::GetOrigin()) {
          renderSelected(player_color);
          return;
        }
      } else {
        player_color = dim(player_color, 94);
      }
      break;
    }
  }

  setColor(player_color);
}

}  // namespace play

}  // namespace state

}  // namespace game