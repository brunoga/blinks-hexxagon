#ifndef GAME_STATE_SETUP_H_
#define GAME_STATE_SETUP_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace setup {

// Setup state handler. Handles initial game configuration.
void Handler(byte state_changed, byte* state, byte* specific_state);

}  // namespace setup

}  // namespace state

}  // namespace game

#endif
