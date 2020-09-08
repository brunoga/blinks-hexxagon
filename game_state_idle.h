#ifndef GAME_STATE_IDLE_H_
#define GAME_STATE_IDLE_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace idle {

// Idle state handler. This just waits for a click to move to the setup state.
void Handler(bool state_changed, byte* state, byte* specific_state);

}  // namespace idle

}  // namespace state

}  // namespace game

#endif