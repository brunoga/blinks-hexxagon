#include "game_state_setup_render.h"

#include "blink_state.h"
#include "game_player.h"

namespace game {

namespace state {

namespace setup {

void Render() {
  Color player_color = game::player::GetColor(blink::state::GetPlayer());

  setColor(dim(player_color, 63));
}

}  // namespace setup

}  // namespace state

}  // namespace game