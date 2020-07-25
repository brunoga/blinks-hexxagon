#ifndef GAME_STATE_IDLE_H_
#define GAME_STATE_IDLE_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace idle {

void Handler(bool state_changed, byte* state, byte* specific_state);
void HandleReceiveMessage(byte message_id, byte* payload);

}  // namespace idle

}  // namespace state

}  // namespace game

#endif