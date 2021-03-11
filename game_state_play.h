#ifndef GAME_STATE_PLAY_H_
#define GAME_STATE_PLAY_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace play {

// Play state handler. Handles actual game play.
void Handler(byte* state, bool button_single_clicked);

}  // namespace play

}  // namespace state

}  // namespace game

#endif