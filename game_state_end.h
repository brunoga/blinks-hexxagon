#ifndef GAME_STATE_END_H_
#define GAME_STATE_END_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace end {

// End state handler. Just let everybody knows who won.
void Handler(byte* state);

}  // namespace end

}  // namespace state

}  // namespace game

#endif