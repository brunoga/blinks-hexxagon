#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "game_state.h"
#include "src/blinks-broadcast/message.h"

#define MESSAGE_GAME_STATE_CHANGE 1
#define MESSAGE_SELECT_ORIGIN 2
#define MESSAGE_SELECT_TARGET 3
#define MESSAGE_EXTERNAL_PROPAGATE_COORDINATES 4  // External handler.
#define MESSAGE_MAP_UPLOAD 6  // Only used for starting the map upload.

namespace game {

namespace message {

void rcv_message_handler(byte message_id, byte src_face, byte* payload,
                         bool loop);

byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                         byte* payload);

bool SendGameStateChange();

bool SendSelectBlink(byte message_id, int8_t x, int8_t y);

bool SendExternalPropagateCoordinates(int8_t x, int8_t y, byte player);

}  // namespace message

}  // namespace game

#endif