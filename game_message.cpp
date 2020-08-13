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

namespace game {

namespace message {

static byte message_state_ = MESSAGE_STATE_SEND_MESSAGE;

static void rcv_message_handler(byte message_id, byte* payload) {
  blink::state::SetColorOverride(true);

  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      game::state::Set(payload[0], true);
      game::state::SetSpecific(payload[1], true);
      game::state::SetPlayer(payload[2]);
      break;
    case MESSAGE_REPORT_BLINK_COUNT:
      game::state::SetBlinkCount(payload);
      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleReceiveMessage(message_id, payload);
      break;
  }
}

static void fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  // blink::state::SetColorOverride(true);

  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleForwardMessage(message_id, src_face, dst_face,
                                              payload);
      break;
  }
}

static game::state::BlinkCount blink_count_;

static void rcv_reply_handler(byte message_id, const byte* payload) {
  // blink::state::SetColorOverride(false);

  switch (message_id) {
    case MESSAGE_CHECK_BOARD: {
      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        blink_count_[i] += payload[i];
      }
      break;
    }
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleReceiveReply(message_id, payload);
      break;
  }
}

static void fwd_reply_handler(byte message_id, byte* payload) {
  // blink::state::SetColorOverride(false);

  switch (message_id) {
    case MESSAGE_CHECK_BOARD: {
      blink_count_[blink::state::GetPlayer()]++;

      for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
        payload[i] = blink_count_[i];
        blink_count_[i] = 0;
      }

      break;
    }
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleForwardReply(message_id, payload);
      break;
  }
}

static bool sendOrWaitForReply(broadcast::Message* message,
                               broadcast::Message* reply) {
  switch (message_state_) {
    case MESSAGE_STATE_SEND_MESSAGE:
      if (broadcast::manager::Send(message)) {
        if (message->header.is_fire_and_forget) return true;

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

bool ReportBlinkCount(game::state::BlinkCount blink_count) {
  broadcast::Message message;

  broadcast::message::Initialize(&message, MESSAGE_REPORT_BLINK_COUNT, true);
  memcpy(message.payload, &blink_count, GAME_PLAYER_MAX_PLAYERS + 1);

  return sendOrWaitForReply(&message, nullptr);
}

bool SendGameStatePlayFindTargets(broadcast::Message* reply) {
  return sendOrWaitForReply(MESSAGE_GAME_STATE_PLAY_FIND_TARGETS, reply);
}

}  // namespace message

}  // namespace game
