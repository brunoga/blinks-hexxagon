#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "manager.h"

namespace game {

namespace state {

namespace idle {

byte Handler(bool state_changed) {
  if (state_changed) {
    game::state::Reset();
    blink::state::Reset();
  }

  if (buttonSingleClicked()) {
    // Switch to setup state.
    return GAME_STATE_SETUP;
  }

  return GAME_STATE_IDLE;
}

void HandleReceiveMessage(byte message_id, byte *payload) {
  (void)message_id;
  (void)payload;
}

}  // namespace idle

}  // namespace state

}  // namespace game
