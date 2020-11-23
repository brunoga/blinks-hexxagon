#ifndef GAME_STATE_SETUP_H_
#define GAME_STATE_SETUP_H_

#include <blinklib.h>

// Play specific states.
#define GAME_STATE_SETUP_SELECT_PLAYERS 0
#define GAME_STATE_SETUP_MAP 1
#define GAME_STATE_SETUP_MAPPED 2
#define GAME_STATE_SETUP_VALIDATE 3

namespace game {

namespace state {

namespace setup {

// Setup state handler. Handles initial game configuration.
void Handler(bool state_changed, byte* state, byte* specific_state);

}  // namespace setup

}  // namespace state

}  // namespace game

#endif
