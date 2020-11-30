#ifndef GAME_STATE_PLAY_H_
#define GAME_STATE_PLAY_H_

#include <blinklib.h>

// Play specific states.
#define GAME_STATE_PLAY_SELECT_ORIGIN 0
#define GAME_STATE_PLAY_ORIGIN_SELECTED 1
#define GAME_STATE_PLAY_SELECT_TARGET 2
#define GAME_STATE_PLAY_TARGET_SELECTED 3
#define GAME_STATE_PLAY_CONFIRM_MOVE 4
#define GAME_STATE_PLAY_MOVE_CONFIRMED 5
#define GAME_STATE_PLAY_RESOLVE_MOVE 6

namespace game {

namespace state {

namespace play {

void Handler(bool state_changed, byte* state, byte* specific_state);

}  // namespace play

}  // namespace state

}  // namespace game

#endif