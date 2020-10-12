#ifndef GAME_STATE_PLAY_H_
#define GAME_STATE_PLAY_H_

#include <blinklib.h>

// Play specific states.
#define GAME_STATE_PLAY_SELECT_ORIGIN 0
#define GAME_STATE_PLAY_SELF_DESTRUCT 1
#define GAME_STATE_PLAY_ORIGIN_SELECTED 2
#define GAME_STATE_PLAY_SELECT_TARGET 3
#define GAME_STATE_PLAY_TARGET_SELECTED 4
#define GAME_STATE_PLAY_CONFIRM_MOVE 5
#define GAME_STATE_PLAY_MOVE_CONFIRMED 6

namespace game {

namespace state {

namespace play {

void Handler(bool state_changed, byte* state, byte* specific_state);

}  // namespace play

}  // namespace state

}  // namespace game

#endif