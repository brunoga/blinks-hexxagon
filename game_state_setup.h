#ifndef GAME_STATE_SETUP_H_
#define GAME_STATE_SETUP_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace setup {

// Setup state handler. Handles game configuration and mapping the board.
void Handler(byte* state, bool button_single_clicked,
             bool button_double_clicked);

}  // namespace setup

}  // namespace state

}  // namespace game

#endif
