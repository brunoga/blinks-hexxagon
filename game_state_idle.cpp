#include "game_state_idle.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "util.h"

#define GAME_STATE_IDLE_WAIT_FOR_BUTTON 0
#define GAME_STATE_IDLE_SEND_RESET 1
#define GAME_STATE_IDLE_WAIT_SENDING_DATAGRAMS 2

namespace game {

namespace state {

namespace idle {

static byte state_;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)specific_state;

  if (state_changed) {
    // We just changed to this state. Reset global (game) and local (Blink)
    // states.
    game::message::Reset();
    game::state::Reset();
    blink::state::Reset();

    state_ = GAME_STATE_IDLE_WAIT_FOR_BUTTON;
  }

  switch (state_) {
    case GAME_STATE_IDLE_WAIT_FOR_BUTTON:
      if (util::NoSleepButtonSingleClicked()) {
        state_ = GAME_STATE_IDLE_SEND_RESET;
      }
      break;
    case GAME_STATE_IDLE_SEND_RESET:
      if (game::message::SendReset()) {
        state_ = GAME_STATE_IDLE_WAIT_SENDING_DATAGRAMS;
      }
      break;
    case GAME_STATE_IDLE_WAIT_SENDING_DATAGRAMS:
      if (!isDatagramPendingOnAnyFace()) {
        state_ = GAME_STATE_IDLE_WAIT_FOR_BUTTON;

        // Switch to setup state.
        *state = GAME_STATE_SETUP;
      }
  }
}

}  // namespace idle

}  // namespace state

}  // namespace game
