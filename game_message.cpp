#include "game_message.h"

#include <string.h>

#include "blink_state.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-broadcast/message.h"
#include "src/blinks-position/position.h"

#define MESSAGE_STATE_SEND_MESSAGE 0
#define MESSAGE_STATE_WAIT_FOR_RESULT 1

// Coordinates stored in the find targets payload.
#define PAYLOAD_X 0
#define PAYLOAD_Y 1

namespace game {

namespace message {

static byte message_state_ = MESSAGE_STATE_SEND_MESSAGE;

static void rcv_message_handler(byte message_id, byte src_face, byte* payload,
                                bool loop) {
  (void)src_face;

  if (loop) {
    // Ignore loops.
    return;
  }

  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      GameStateChangeData data;
      data.value = payload[0];

      game::state::Set(data.state, true);
      game::state::SetSpecific(data.specific_state, true);
      game::state::SetPlayer(data.next_player + 1);
      break;
    case MESSAGE_SELECT_ORIGIN: {
      if (blink::state::GetPlayer() != 0) break;

      if (position::Distance(position::Coordinates{
              (int8_t)payload[PAYLOAD_X], (int8_t)payload[PAYLOAD_Y]}) <= 2) {
        blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_TARGET);
      }
      break;
    }
    case MESSAGE_FLASH:
      blink::state::StartColorOverride();
      break;
  }
}  // namespace message

static byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  (void)src_face;
  (void)dst_face;
  (void)payload;

  byte len = 0;

  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      len = 1;
      break;
    case MESSAGE_SELECT_ORIGIN:
      len = 2;
      break;
  }

  return len;
}

static bool sendOrWaitForReply(broadcast::Message* message,
                               broadcast::Message* reply) {
  switch (message_state_) {
    case MESSAGE_STATE_SEND_MESSAGE:
      if (broadcast::manager::Send(message)) {
        if (message->header.is_fire_and_forget) {
          return true;
        }

        message_state_ = MESSAGE_STATE_WAIT_FOR_RESULT;
      }

      break;
    case MESSAGE_STATE_WAIT_FOR_RESULT:
      if (broadcast::manager::Receive(reply)) {
        message_state_ = MESSAGE_STATE_SEND_MESSAGE;

        return true;
      }
  }

  return false;
}

static bool sendOrWaitForReply(byte message_id, const byte* payload,
                               byte payload_size, broadcast::Message* reply) {
  broadcast::Message message;
  broadcast::message::Initialize(&message, message_id, reply == nullptr);

  if (payload != nullptr) {
    memcpy(message.payload, payload, payload_size);
  }

  return sendOrWaitForReply(&message, reply);
}

void Setup() {
  broadcast::manager::Setup(rcv_message_handler, fwd_message_handler, nullptr,
                            nullptr);
}

void Process() { broadcast::manager::Process(); }

void Reset() { message_state_ = MESSAGE_STATE_SEND_MESSAGE; }

bool SendGameStateChange(byte payload) {
  return sendOrWaitForReply(MESSAGE_GAME_STATE_CHANGE, &payload, 1, nullptr);
}

bool SendSelectOrigin(int8_t x, int8_t y) {
  byte payload[2] = {(byte)x, (byte)y};
  return sendOrWaitForReply(MESSAGE_SELECT_ORIGIN, payload, 2, nullptr);
}

bool SendFlash() {
  if (sendOrWaitForReply(MESSAGE_FLASH, nullptr, 0, nullptr)) {
    blink::state::StartColorOverride();

    return true;
  }

  return false;
}

bool SendReset() {
  return sendOrWaitForReply(MESSAGE_RESET, nullptr, 0, nullptr);
}

}  // namespace message

}  // namespace game
