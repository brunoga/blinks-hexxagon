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

static bool auto_select_ = false;

static void select_origin(byte* state, byte* specific_state) {
  LOGFLN("select origin");
  // We are going to select an origin, so reset any blink that is currently one.
  blink::state::SetOrigin(false);

  // Also, if there is a target selected. It must be reset (as we have no
  // origin).
  blink::state::SetTarget(false);

  // Also reset any potential targets.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  // This blink does not belong to a player. Nothing to do.
  if (blink::state::GetType() != BLINK_STATE_TYPE_PLAYER) return;

  // It belongs to a player, but not the current one. Nothing to do.
  if (blink::state::GetPlayer() != game::state::GetPlayer()) return;

  // We pass all checks, but we do nothing until we get a click.
  if (!buttonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  blink::state::SetOrigin(true);

  // Indicate that an origin was selected. This will be automatically propagated
  // to all blinks.
  *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
}

static void origin_selected(byte* state, byte* specific_state) {
  LOGFLN("origin selected");
  // Only the origin blink has anything to do here.
  if (!blink::state::GetOrigin()) return;

  // Look for possible targets. Only continue when all other blinks report in.
  broadcast::message::Message reply;
  if (!game::message::SendGameStatePlayFindTargets(reply)) return;

  // Did we find any targets?
  if (broadcast::message::Payload(reply)[0] == 0) {
    // Nope. Reset to selecting an origin (hoppefully a different one).
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We have at least one possible target.
  *specific_state = GAME_STATE_PLAY_SELECT_TARGET;
}

static void select_target(byte* state, byte* specific_state) {
  LOGFLN("select target");
  // We are going to select a target, so reset any blink that is currently one.
  blink::state::SetTarget(false);

  // If this blink is the origin, there is nothing for it to do at this stage.
  if (blink::state::GetOrigin()) return;

  // If we are not a possible target or a blink that belongs to the current
  // player, then there is also nothing to do.
  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE &&
      (blink::state::GetType() != BLINK_STATE_TYPE_PLAYER ||
       blink::state::GetPlayer() != game::state::GetPlayer())) {
    return;
  }

  // We pass all checks, but we do nothing until we get a click or auto
  // selection is enabled for this blink.
  if (!buttonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Are we a blink that belongs to the current player?
  if ((blink::state::GetType() == BLINK_STATE_TYPE_PLAYER) &&
      (blink::state::GetPlayer() == game::state::GetPlayer())) {
    auto_select_ = true;

    // Change our state accordingly.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We are a valid target.
  blink::state::SetTarget(true);

  *specific_state = GAME_STATE_PLAY_TARGET_SELECTED;
}

static void target_selected(byte* state, byte* specific_state) {
  LOGFLN("target selected");
  // Double-click in any blink confirms the move.
  if (buttonDoubleClicked()) {
    blink::state::SetArbitrator(true);

    *specific_state = GAME_STATE_PLAY_CONFIRM_MOVE;
    return;
  }

  // If this blink is the current target, there is nothing for it to do at this
  // stage.
  if (blink::state::GetTarget()) return;

  // If this blink is the current origin, there is nothing for it to do at this
  // stage.
  if (blink::state::GetOrigin()) return;

  // If we are not a possible target or a blink that belongs to the current
  // player, then there is also nothing to do.
  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE &&
      (blink::state::GetType() != BLINK_STATE_TYPE_PLAYER ||
       blink::state::GetPlayer() != game::state::GetPlayer())) {
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!buttonSingleClicked()) return;

  // Are we a blink that belong to the current player?
  if ((blink::state::GetType() == BLINK_STATE_TYPE_PLAYER) &&
      (blink::state::GetPlayer() == game::state::GetPlayer())) {
    // Yes. We are a new origin now.
    auto_select_ = true;

    // Change our state accordingly.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
    return;
  }

  // Are we a different target?
  if (blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_NONE) {
    // Yes. Set ourselves as the target.
    auto_select_ = true;

    *specific_state = GAME_STATE_PLAY_SELECT_TARGET;
  }
}

static bool neighboor_target() {
  FOREACH_FACE(f) {
    if (isValueReceivedOnFaceExpired(f)) continue;

    blink::state::FaceValue face_value;
    face_value.value = getLastValueReceivedOnFace(f);

    if (face_value.target) return true;
  }

  return false;
}

static void confirm_move(byte* state, byte* specific_state) {
  LOGFLN("confirm move");
  if (blink::state::GetOrigin()) {
    LOGFLN("origin");
    if (!neighboor_target()) {
      LOGFLN("target not neighboor");
      // We are the origin and the target is not an immediate neighboor. We are
      // moving from here so reset ourselves.
      blink::state::Reset();
    } else {
      LOGFLN("target neighboor");
    }
  } else if (blink::state::GetTarget()) {
    // We are the target. Become a player blink.
    //
    // TODO(bga): We do not remove the target flag here so it will still be
    // present when we are reading the face value in a origin neighboor blink.
    // There is most likelly a better way to do this.
    blink::state::SetType(BLINK_STATE_TYPE_PLAYER);
    blink::state::SetPlayer(game::state::GetPlayer());
  }

  // Clear target type for everybody.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  if (!blink::state::GetArbitrator()) return;

  *specific_state = GAME_STATE_PLAY_MOVE_CONFIRMED;
}

static void move_confirmed(byte* state, byte* specific_state) {
  LOGFLN("move confirmed");

  // Now it is ok to clear the target flag.
  if (blink::state::GetTarget()) blink::state::SetTarget(false);

  if (!blink::state::GetArbitrator()) return;

  broadcast::message::Message reply;
  if (!game::message::SendCheckBoard(reply)) {
    return;
  }

  const byte* payload = broadcast::message::Payload(reply);
  byte empty_blinks = payload[0] - (payload[1] + payload[2]);
  if (payload[1] == 0 || payload[2] == 0 || empty_blinks == 0) {
    // One of the players has zero blinks or there is zero spaces left. Game
    // over.
    *state = GAME_STATE_END;
    *specific_state = 0;

    return;
  }

  byte next_player = game::state::GetPlayer() + 1;

  game::state::SetPlayer(next_player > 2 ? 1 : next_player);

  blink::state::SetArbitrator(false);

  *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    *state = GAME_STATE_PLAY;
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    game::state::SetPlayer(1);

    return;
  }

  switch (game::state::GetSpecific()) {
    case GAME_STATE_PLAY_SELECT_ORIGIN:
      select_origin(state, specific_state);
      break;
    case GAME_STATE_PLAY_ORIGIN_SELECTED:
      origin_selected(state, specific_state);
      break;
    case GAME_STATE_PLAY_SELECT_TARGET:
      select_target(state, specific_state);
      break;
    case GAME_STATE_PLAY_TARGET_SELECTED:
      target_selected(state, specific_state);
      break;
    case GAME_STATE_PLAY_CONFIRM_MOVE:
      confirm_move(state, specific_state);
      break;
    case GAME_STATE_PLAY_MOVE_CONFIRMED:
      move_confirmed(state, specific_state);
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
