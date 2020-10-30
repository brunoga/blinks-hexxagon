#include "game_message.h"

#include <string.h>

#include "blink_state.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-broadcast/message.h"

#define MESSAGE_STATE_SEND_MESSAGE 0
#define MESSAGE_STATE_WAIT_FOR_RESULT 1

// Coordinates stored in find targets the payload.
#define PAYLOAD_X 0
#define PAYLOAD_Y 1
#define PAYLOAD_Z 2

// Face the find targets payload was originally sent to.
#define PAYLOAD_FACE 3

namespace game {

namespace message {

static const byte opposite_face_[] = {3, 4, 5, 0, 1, 2};

static byte message_state_ = MESSAGE_STATE_SEND_MESSAGE;

static byte abs(int8_t i) { return i < 0 ? -i : i; }

static void set_payload_for_face(byte* payload, byte f) {
  switch (f) {
    case 0:
      payload[PAYLOAD_Y]--;
      payload[PAYLOAD_Z]--;
      break;
    case 1:
      payload[PAYLOAD_X]--;
      payload[PAYLOAD_Y]--;
      break;
    case 2:
      payload[PAYLOAD_X]--;
      payload[PAYLOAD_Z]++;
      break;
    case 3:
      payload[PAYLOAD_Y]++;
      payload[PAYLOAD_Z]++;
      break;
    case 4:
      payload[PAYLOAD_X]++;
      payload[PAYLOAD_Y]++;
      break;
    case 5:
      payload[PAYLOAD_X]++;
      payload[PAYLOAD_Z]--;
  }

  payload[PAYLOAD_FACE] = f;
}

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
    case MESSAGE_FIND_TARGETS:
      if (blink::state::GetPlayer() != 0) break;

      if (abs(int8_t(payload[0])) <= 2 && abs(int8_t(payload[1])) <= 2 &&
          abs(int8_t(payload[2])) <= 2) {
        blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_TARGET);
      }
      break;
    case MESSAGE_REPORT_BOARD_STATE:
      game::state::SetBlinkCount(payload);
      break;
    case MESSAGE_FLASH:
      blink::state::StartColorOverride();
      break;
  }
}

static byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  (void)src_face;

  byte len = MESSAGE_PAYLOAD_BYTES;

  switch (message_id) {
    case MESSAGE_FIND_TARGETS:
      if (src_face == FACE_COUNT) {
        // We are the source of the coordinate system. Set payload using the
        // real face number.
        set_payload_for_face(payload, dst_face);
      } else {
        // The input face is the face opposite to the face the message was
        // sent to.
        byte input_face = opposite_face_[payload[3]];

        // The output face is the normalized face in relation to the input
        // face.
        byte output_face =
            ((dst_face - src_face) + input_face + FACE_COUNT) % FACE_COUNT;

        // Set payload using the normalized output face.
        set_payload_for_face(payload, output_face);
      }

      len = 4;
      break;
    case MESSAGE_GAME_STATE_CHANGE:
      len = 1;
      break;
    case MESSAGE_CHECK_BOARD_STATE:
    case MESSAGE_FLASH:
      len = 0;
      break;
  }

  return len;
}

static byte blink_count_[GAME_PLAYER_MAX_PLAYERS + 1];
static byte upstream_target_;

static void rcv_reply_handler(byte message_id, byte src_face,
                              const byte* payload) {
  (void)src_face;

  switch (message_id) {
    case MESSAGE_CHECK_BOARD_STATE:
      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        blink_count_[i] += payload[i];
      }
      break;

    case MESSAGE_FIND_TARGETS:
      if (payload[0] != 0) {
        upstream_target_ = true;
      }
      break;
  }
}

static byte fwd_reply_handler(byte message_id, byte dst_face, byte* payload) {
  (void)dst_face;

  byte len = MESSAGE_PAYLOAD_BYTES;

  switch (message_id) {
    case MESSAGE_CHECK_BOARD_STATE:
      blink_count_[blink::state::GetPlayer()]++;

      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        payload[i] = blink_count_[i];
        blink_count_[i] = 0;
      }

      len = GAME_PLAYER_MAX_PLAYERS + 1;
      break;
    case MESSAGE_FIND_TARGETS:
      if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_TARGET ||
          upstream_target_) {
        // Just indicate in the payload that we are or we know a target.
        payload[0] = 1;
      }

      upstream_target_ = false;

      len = 1;
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
  broadcast::manager::Setup(rcv_message_handler, fwd_message_handler,
                            rcv_reply_handler, fwd_reply_handler);
}

void Process() { broadcast::manager::Process(); }

bool SendGameStateChange(byte payload) {
  return sendOrWaitForReply(MESSAGE_GAME_STATE_CHANGE, &payload, 1, nullptr);
}

bool SendCheckBoardState(broadcast::Message* reply) {
  return sendOrWaitForReply(MESSAGE_CHECK_BOARD_STATE, nullptr, 0, reply);
}

bool SendReportBoardState() {
  return sendOrWaitForReply(MESSAGE_REPORT_BOARD_STATE,
                            game::state::GetBlinkCount(),
                            GAME_PLAYER_MAX_PLAYERS + 1, nullptr);
}

bool SendFindTargets(broadcast::Message* reply) {
  return sendOrWaitForReply(MESSAGE_FIND_TARGETS, nullptr, 0, reply);
}

bool SendFlash() {
  if (sendOrWaitForReply(MESSAGE_FLASH, nullptr, 0, nullptr)) {
    blink::state::StartColorOverride();

    return true;
  }

  return false;
}

}  // namespace message

}  // namespace game
