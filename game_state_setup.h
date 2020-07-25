#ifndef GAME_STATE_SETUP_H_
#define GAME_STATE_SETUP_H_

#include <blinklib.h>

namespace game {

namespace state {

namespace setup {

void Handler(bool state_changed, byte* state, byte* specific_state);
void HandleReceiveMessage(byte message_id, byte* payload);

}  // namespace setup

}  // namespace state

}  // namespace game

#endif
