#include "game_state_end.h"

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace end {

void Handler(byte* state, bool button_double_clicked) {
  if (!button_double_clicked) return;

  *state = GAME_STATE_IDLE;
  blink::state::face::handler::ResetGame();
}

}  // namespace end

}  // namespace state

}  // namespace game