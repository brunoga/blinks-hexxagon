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
  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      game::state::Set(payload[0], true);
      game::state::SetSpecific(payload[1], true);
      game::state::SetNextPlayer(payload[2]);
      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleReceiveMessage(message_id, payload);
      break;
  }
}

static void fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleForwardMessage(message_id, src_face, dst_face,
                                              payload);
      break;
  }
}

static byte num_blinks_ = 0;
static byte num_player_one_ = 0;
static byte num_player_two_ = 0;

static void rcv_reply_handler(byte message_id, const byte* payload) {
  switch (message_id) {
    case MESSAGE_CHECK_BOARD:
      num_blinks_ += payload[0];
      num_player_one_ += payload[1];
      num_player_two_ += payload[2];
      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleReceiveReply(message_id, payload);
      break;
  }
}

static void fwd_reply_handler(byte message_id, byte* payload) {
  switch (message_id) {
    case MESSAGE_CHECK_BOARD:
      if (blink::state::GetType() == BLINK_STATE_TYPE_PLAYER) {
        if (blink::state::GetPlayer() == 1) {
          num_player_one_++;
        } else {
          num_player_two_++;
        }
      }
      num_blinks_++;

      payload[0] = num_blinks_;
      payload[1] = num_player_one_;
      payload[2] = num_player_two_;

      num_blinks_ = 0;
      num_player_one_ = 0;
      num_player_two_ = 0;

      break;
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      game::state::play::HandleForwardReply(message_id, payload);
      break;
  }
}

static bool sendOrWaitForReply(broadcast::message::Message message,
                               broadcast::message::Message reply) {
  switch (message_state_) {
    case MESSAGE_STATE_SEND_MESSAGE:
      if (broadcast::manager::Send(message)) {
        message_state_ = MESSAGE_STATE_WAIT_FOR_RESULT;
      }

      return false;
    case MESSAGE_STATE_WAIT_FOR_RESULT:
      if (broadcast::manager::Receive(reply)) {
        message_state_ = MESSAGE_STATE_SEND_MESSAGE;

        return true;
      }

      return false;
  }

  return false;
}

static bool sendOrWaitForReply(byte message_id, const byte* payload,
                               broadcast::message::Message reply) {
  broadcast::message::Message message;
  broadcast::message::Set(message, message_id, payload, false);

  return sendOrWaitForReply(message, reply);
}

void Setup() {
  broadcast::manager::Setup(rcv_message_handler, fwd_message_handler,
                            rcv_reply_handler, fwd_reply_handler);
}

void Process() { ::broadcast::manager::Process(); }

bool SendGameStateChange(byte game_state, byte specific_state,
                         byte next_player) {
  broadcast::message::Message message;
  broadcast::message::Set(message, MESSAGE_GAME_STATE_CHANGE, nullptr, false);

  broadcast::message::MutablePayload(message)[0] = game_state;
  broadcast::message::MutablePayload(message)[1] = specific_state;
  broadcast::message::MutablePayload(message)[2] = next_player;

  return sendOrWaitForReply(message, nullptr);
}

bool SendCheckBoard(broadcast::message::Message reply) {
  return sendOrWaitForReply(MESSAGE_CHECK_BOARD, nullptr, reply);
}

bool SendGameStatePlayFindTargets(broadcast::message::Message reply) {
  return sendOrWaitForReply(MESSAGE_GAME_STATE_PLAY_FIND_TARGETS, nullptr,
                            reply);
}

}  // namespace message

}  // namespace game
