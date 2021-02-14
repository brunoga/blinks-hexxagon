#include "game_state_setup_render.h"

#include <hexxagon_config.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_setup.h"

namespace game {

namespace state {

namespace setup {

void Render() {
  if (game::state::Get() != GAME_STATE_SETUP_MAP) {
    Color player_color = game::player::GetColor(blink::state::GetPlayer());

    if (blink::state::GetPlayer() == GAME_PLAYER_NO_PLAYER) {
      blink::state::render::Empty(HEXXAGON_RENDER_SETUP_STATE_COLOR_DIM);
    } else {
      setColor(dim(player_color, HEXXAGON_RENDER_SETUP_STATE_COLOR_DIM));
    }
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game