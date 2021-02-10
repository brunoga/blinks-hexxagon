#ifndef GAME_STATE_PLAY_H_
#define GAME_STATE_PLAY_H_

#include <blinklib.h>

#include "blink_state_face.h"

namespace game {

namespace state {

namespace play {

// Play state handler. Handles actual game play.
void Handler(byte* state,
             const blink::state::face::ValueHandler& face_value_handler);

}  // namespace play

}  // namespace state

}  // namespace game

#endif