#include "game_state_setup_render.h"

#include "blink_state.h"

namespace game {

namespace state {

namespace setup {

void Render() {
  Color color;

  // TODO(bga): Extend to 4 players.
  switch (blink::state::GetPlayer()) {
    case 0:
      color = dim(ORANGE, 63);
      break;
    case 1:
      color = dim(RED, 63);
      break;
    case 2:
      color = dim(BLUE, 63);
      break;
  }

  setColor(color);
}

}  // namespace setup

}  // namespace state

}  // namespace game