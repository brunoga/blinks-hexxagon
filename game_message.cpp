#include "game_message.h"

#include <string.h>

#include "blink_state.h"
#include "debug.h"
#include "game_state.h"
#include "game_state_play.h"
#include "manager.h"
#include "message.h"

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

static void rcv_message_handler(byte message_id, byte* payload) {
  blink::state::SetColorOverride(true);

  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      game::state::Set(payload[0], true);
      game::state::SetSpecific(payload[1], true);
      game::state::SetPlayer(payload[2]);
      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (blink::state::GetPlayer() != 0) break;

      if (abs(int8_t(payload[0])) <= 2 && abs(int8_t(payload[1])) <= 2 &&
          abs(int8_t(payload[2])) <= 2) {
        blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_TARGET);
      }
      break;
    case MESSAGE_REPORT_WINNER:
      blink::state::SetPlayer(payload[0]);
      break;
  }
}

static byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  blink::state::SetColorOverride(true);

  byte len = MESSAGE_PAYLOAD_BYTES;

  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (src_face == FACE_COUNT) {
        // We are the source of the coordinate system. Set payload using the
        // real face number.
        set_payload_for_face(payload, dst_face);
      } else {
        // The input face is the face opposite to the face the message was sent
        // to.
        byte input_face = ((payload[3] + FACE_COUNT / 2) % FACE_COUNT);

        // The output face is the normalized face in relation to the input face.
        byte output_face =
            ((dst_face - src_face) + input_face + FACE_COUNT) % FACE_COUNT;

        // Set payload using the normalized output face.
        set_payload_for_face(payload, output_face);
      }

      len = 4;
      break;
    case MESSAGE_GAME_STATE_CHANGE:
      len = 3;
      break;
    case MESSAGE_CHECK_BOARD:
      len = 0;
      break;
    case MESSAGE_REPORT_WINNER:
      len = 1;
      break;
  }

  return len;
}

static game::state::BlinkCount blink_count_;
static byte upstream_target_;

static void rcv_reply_handler(byte message_id, const byte* payload) {
  blink::state::SetColorOverride(false);

  switch (message_id) {
    case MESSAGE_CHECK_BOARD:
      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        blink_count_[i] += payload[i];
      }
      break;

    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (payload[0] != 0) {
        upstream_target_ = true;
      }
      break;
  }
}

static byte fwd_reply_handler(byte message_id, byte* payload) {
  blink::state::SetColorOverride(false);

  byte len = MESSAGE_PAYLOAD_BYTES;

  switch (message_id) {
    case MESSAGE_CHECK_BOARD:
      blink_count_[blink::state::GetPlayer()]++;

      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        payload[i] = blink_count_[i];
        blink_count_[i] = 0;
      }

      len = GAME_PLAYER_MAX_PLAYERS + 1;
      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_TARGET ||
          upstream_target_) {
        // Just indicate in the payload that we are or we know a target.
        payload[0] = 1;
      }

      upstream_target_ = false;

      len = 1;
      break;
    case MESSAGE_GAME_STATE_CHANGE:
    case MESSAGE_REPORT_WINNER:
      len = 0;
      break;
  }

  return len;
}

static bool sendOrWaitForReply(broadcast::Message* message,
                               broadcast::Message* reply) {
  switch (message_state_) {
    case MESSAGE_STATE_SEND_MESSAGE:
      if (broadcast::manager::Send(message)) {
        message_state_ = MESSAGE_STATE_WAIT_FOR_RESULT;
      }

      break;
    case MESSAGE_STATE_WAIT_FOR_RESULT:
      if (broadcast::manager::Receive(reply)) {
        message_state_ = MESSAGE_STATE_SEND_MESSAGE;

        return true;
      }

      break;
  }

  return false;
}

static bool sendOrWaitForReply(byte message_id, broadcast::Message* reply) {
  broadcast::Message message;
  broadcast::message::Initialize(&message, message_id, false);

  return sendOrWaitForReply(&message, reply);
}

void Setup() {
  broadcast::manager::Setup(rcv_message_handler, fwd_message_handler,
                            rcv_reply_handler, fwd_reply_handler);
}

void Process() { broadcast::manager::Process(); }

bool SendGameStateChange(byte game_state, byte specific_state,
                         byte next_player) {
  broadcast::Message message;

  broadcast::message::Initialize(&message, MESSAGE_GAME_STATE_CHANGE, false);
  message.payload[0] = game_state;
  message.payload[1] = specific_state;
  message.payload[2] = next_player;

  return sendOrWaitForReply(&message, nullptr);
}

bool SendCheckBoard(broadcast::Message* reply) {
  return sendOrWaitForReply(MESSAGE_CHECK_BOARD, reply);
}

bool SendGameStatePlayFindTargets(broadcast::Message* reply) {
  return sendOrWaitForReply(MESSAGE_GAME_STATE_PLAY_FIND_TARGETS, reply);
}

bool SendReportWinner(byte winner_player) {
  broadcast::Message message;

  broadcast::message::Initialize(&message, MESSAGE_REPORT_WINNER, false);
  message.payload[0] = winner_player;

  return sendOrWaitForReply(&message, nullptr);
}

}  // namespace message

}  // namespace game
