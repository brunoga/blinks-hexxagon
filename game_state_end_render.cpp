#include "game_state_end_render.h"

#include <hexxagon_config.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_map.h"
#include "game_player.h"

namespace game {

namespace state {

namespace end {

void Render() {
  byte player = blink::state::GetPlayer();
  if (player != GAME_PLAYER_NO_PLAYER) {
    bool is_winner =
        ((1 << player) & game::map::GetStatistics().winning_players_mask);
    if (!is_winner) {
      // Losing player. Pieces should "explode".
      if (blink::state::render::Explosion(
              game::player::GetColor(GAME_PLAYER_NO_PLAYER))) {
        blink::state::SetPlayer(GAME_PLAYER_NO_PLAYER);
      }

      return;
    }
  }

  blink::state::render::Pulse(HEXXAGON_RENDER_END_STATE_PULSE_START_DIM,
                              HEXXAGON_RENDER_END_STATE_PULSE_SLOWDOWN);
}

}  // namespace end

}  // namespace state

}  // namespace game