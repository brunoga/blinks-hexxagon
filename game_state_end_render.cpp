#include "game_state_end_render.h"

#include "blink_state.h"
#include "game_player.h"

namespace game {

namespace state {

namespace end {

void Render() {
  Color player_color = game::player::GetColor(blink::state::GetPlayer());

  setColor(player_color);
}

}  // namespace end

}  // namespace state

}  // namespace game