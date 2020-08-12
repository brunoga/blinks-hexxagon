#include "game_state_setup_render.h"

#include "blink_state.h"
#include "game_player.h"

namespace game {

namespace state {

namespace setup {

void Render() {
  Color current_player_color =
      game::player::GetColor(blink::state::GetPlayer());

  setColor(dim(current_player_color, 63));
}

}  // namespace setup

}  // namespace state

}  // namespace game