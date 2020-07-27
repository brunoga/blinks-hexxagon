#include "game_state_play_render.h"

#include "blink_state.h"
#include "debug.h"
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
  Color color;

  switch (blink::state::GetType()) {
    case BLINK_STATE_TYPE_EMPTY:
      if (blink::state::GetTarget()) {
        color = ORANGE;
        renderSelected(color);
        return;
      }

      if (blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_NONE) {
        color = dim(ORANGE, 191);
      } else {
        color = dim(ORANGE, 127);
      }
      break;
    case BLINK_STATE_TYPE_PLAYER: {
      if (blink::state::GetPlayer() == 1) {
        color = RED;
      } else {
        color = BLUE;
      }

      if (game::state::GetPlayer() == blink::state::GetPlayer()) {
        if (!blink::state::GetOrigin()) {
          color = dim(color, 191);
        } else {
          renderSelected(color);
          return;
        }
      } else {
        color = dim(color, 127);
      }
      break;
    }
  }

  setColor(color);
}

}  // namespace play

}  // namespace state

}  // namespace game