#ifndef GAME_STATE_IDLE_H_
#define GAME_STATE_IDLE_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace idle {

byte Handler(bool state_changed);
void HandleReceiveMessage(byte message_id, byte *payload);

}  // namespace idle

}  // namespace state

}  // namespace game

#endif