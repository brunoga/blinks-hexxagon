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
  if (game::state::Get() == GAME_STATE_SETUP_MAP) {
    blink::state::render::Pulse(HEXXAGON_RENDER_SETUP_STATE_PULSE_START_DIM,
                                HEXXAGON_RENDER_SETUP_STATE_PULSE_SLOWDOWN);
  } else {
    byte brightness = blink::state::GetPlayer() == GAME_PLAYER_NO_PLAYER
                          ? HEXXAGON_RENDER_SETUP_STATE_COLOR_DIM
                          : 255 - HEXXAGON_RENDER_DIM_ADJUSTMENT;
    blink::state::render::Player(brightness);
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game