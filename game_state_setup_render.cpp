#include "game_state_setup_render.h"

#include "blink_state.h"

namespace game {

namespace state {

namespace setup {

void Render() {
  Color color;

  switch (blink::state::GetPlayer()) {
    case 0:
      color = dim(ORANGE, 63);
      break;
    default: {
      if (blink::state::GetPlayer() == 1) {
        color = dim(RED, 63);
      } else {
        color = dim(BLUE, 63);
      }

      break;
    }
  }

  setColor(color);
}

}  // namespace setup

}  // namespace state

}  // namespace game