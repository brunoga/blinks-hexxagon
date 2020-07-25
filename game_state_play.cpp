#include "game_state_play.h"

#include <Arduino.h>  // for abs

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "message.h"

// Coordinates stored in the payload.
#define PAYLOAD_X 0
#define PAYLOAD_Y 1
#define PAYLOAD_Z 2

// Face the payload was originally sent to.
#define PAYLOAD_FACE 3

namespace game {

namespace state {

namespace play {

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

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    *state = GAME_STATE_PLAY;
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    game::state::SetNextPlayer(1);

    return;
  }

  switch (game::state::GetSpecific()) {
    case GAME_STATE_PLAY_SELECT_ORIGIN:
      LOGFLN("select origin");
      blink::state::SetOrigin(false);
      blink::state::SetTarget(false);
      blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

      if (blink::state::GetType() != BLINK_STATE_TYPE_PLAYER) break;

      if (blink::state::GetPlayer() != game::state::GetNextPlayer()) break;

      if (!buttonSingleClicked()) break;

      blink::state::SetOrigin(true);

      *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
      break;
    case GAME_STATE_PLAY_ORIGIN_SELECTED: {
      LOGFLN("origin selected");
      blink::state::SetTarget(false);
      // blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

      if (!blink::state::GetOrigin()) break;

      LOGFLN("finding targets");

      broadcast::message::Message reply;
      if (!game::message::SendGameStatePlayFindTargets(reply)) break;

      LOGFLN("done");

      if (broadcast::message::Payload(reply)[0] == 0) {
        // No targets.
        LOGFLN("no targets");
        *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

        break;
      } else {
        LOGFLN("got targets");
      }

      *specific_state = GAME_STATE_PLAY_SELECT_TARGET;
      break;
    }
    case GAME_STATE_PLAY_SELECT_TARGET:
      LOGFLN("select target");
      blink::state::SetTarget(false);

      if (blink::state::GetOrigin()) break;

      if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE &&
          blink::state::GetType() != BLINK_STATE_TYPE_PLAYER &&
          blink::state::GetPlayer() != game::state::GetNextPlayer()) {
        break;
      }

      if (!buttonSingleClicked()) break;

      if (blink::state::GetPlayer() == game::state::GetNextPlayer()) {
        blink::state::SetOrigin(true);
        *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
      }

      blink::state::SetTarget(true);

      *specific_state = GAME_STATE_PLAY_TARGET_SELECTED;
      break;
    case GAME_STATE_PLAY_TARGET_SELECTED:
      if (buttonDoubleClicked()) {
        *specific_state = GAME_STATE_PLAY_CONFIRM_MOVE;

        break;
      }

      if (!buttonSingleClicked()) break;

      if (blink::state::GetOrigin()) {
        // Unselect origin.
        *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

        break;
      }

      if (blink::state::GetTarget()) {
        // Unselect origin.
        *specific_state = GAME_STATE_PLAY_SELECT_TARGET;

        break;
      }

      break;
    case GAME_STATE_PLAY_CONFIRM_MOVE:
      break;
  }

  *state = GAME_STATE_PLAY;
}

void HandleReceiveMessage(byte message_id, byte* payload) {
  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS: {
      if (blink::state::GetType() == BLINK_STATE_TYPE_EMPTY) {
        byte selected_target_type = BLINK_STATE_TARGET_TYPE_NONE;
        for (byte i = 0; i < 3; ++i) {
          byte distance = abs(int8_t(payload[i]));
          if (distance > 2) {
            selected_target_type = BLINK_STATE_TARGET_TYPE_NONE;
            break;
          }

          if (distance > selected_target_type) {
            selected_target_type = distance;
          }
        }

        LOGF("target type ");
        LOGLN(selected_target_type);

        blink::state::SetTargetType(selected_target_type);
      }
      break;
    }
  }
}

void HandleForwardMessage(byte message_id, byte src_face, byte dst_face,
                          byte* payload) {
  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS: {
      if (src_face == FACE_COUNT) {
        // We are the source of the coordinate system. Set payload using the
        // real face number.
        set_payload_for_face(payload, dst_face);

        return;
      }

      // The input face is the face opposite to the face the message was sent
      // to.
      byte input_face = ((payload[3] + 3) % 6);

      // The output face is the normalized face in relation to the input face.
      byte output_face = ((dst_face - src_face) + input_face + 6) % 6;

      // Set payload using the normalized output face.
      set_payload_for_face(payload, output_face);
    }
  }
}

static byte upstream_target_ = false;

void HandleReceiveReply(byte message_id, const byte* payload) {
  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (payload[0] != 0) {
        upstream_target_ = true;
      }
  }
}

void HandleForwardReply(byte message_id, byte* payload) {
  switch (message_id) {
    case MESSAGE_GAME_STATE_PLAY_FIND_TARGETS:
      if (blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_NONE ||
          upstream_target_) {
        // Just indicate in the payload that we have a target.
        payload[0] = 1;
      }

      upstream_target_ = false;
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
