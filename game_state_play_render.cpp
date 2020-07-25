#include "game_state_play_render.h"

#include "blink_state.h"
#include "game_state.h"

namespace game {

namespace state {

namespace play {

void Render() {
  Color color;

  switch (blink::state::GetType()) {
    case BLINK_STATE_TYPE_EMPTY:
      if (blink::state::GetTarget()) {
        color = ORANGE;
        break;
      }

      if (blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_NONE) {
        color = dim(ORANGE, 127);
      } else {
        color = dim(ORANGE, 63);
      }
      break;
    case BLINK_STATE_TYPE_PLAYER: {
      if (blink::state::GetPlayer() == 1) {
        color = RED;
      } else {
        color = BLUE;
      }

      if (game::state::GetNextPlayer() == blink::state::GetPlayer()) {
        if (!blink::state::GetOrigin()) {
          color = dim(color, 127);
        }
      } else {
        color = dim(color, 63);
      }
      break;
    }
  }

  setColor(color);
}

}  // namespace play

}  // namespace state

}  // namespace game