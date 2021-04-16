#ifndef BROADCAST_CONFIG_H_
#define BROADCAST_CONFIG_H_

namespace broadcast {

struct Message;

}

namespace game {

namespace map {

bool external_message_handler(byte local_absolute_face,
                              const broadcast::Message* message);
}  // namespace map

namespace message {

void rcv_message_handler(byte message_id, byte src_face, byte* payload,
                         bool loop);

byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                         byte* payload);

}  // namespace message

}  // namespace game

// This is the maximum payload we will need.
#define BROADCAST_MESSAGE_PAYLOAD_BYTES 4

// Disable message replies. Saves considerable space.
#define BROADCAST_DISABLE_REPLIES

// Setup message handlers.
#define BROADCAST_EXTERNAL_MESSAGE_HANDLER game::map::external_message_handler

#define BROADCAST_RCV_MESSAGE_HANDLER game::message::rcv_message_handler
#define BROADCAST_FWD_MESSAGE_HANDLER game::message::fwd_message_handler

#endif
