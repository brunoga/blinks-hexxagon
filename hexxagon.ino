#include "manager.h"
#include "message.h"

#define MESSAGE_FIND_TARGETS 1

#define TARGET_TYPE_NONE 0
#define TARGET_TYPE_MOVE 1
#define TARGET_TYPE_COPY 2

// Coordinates stored in the payload.
#define PAYLOAD_X 0
#define PAYLOAD_Y 1
#define PAYLOAD_Z 2

// Face the payload was originally sent to.
#define PAYLOAD_FACE 3

byte target_type = TARGET_TYPE_NONE;

Color displayColor = OFF;

void rcv_message_handler(byte message_id, byte* payload) {
  byte selected_target_type = TARGET_TYPE_NONE;
  for (byte i = 0; i < 3; ++i) {
    byte distance = abs(int8_t(payload[i]));
    if (distance > 2) {
      selected_target_type = TARGET_TYPE_NONE;
      break;
    }

    if (distance > selected_target_type) {
      selected_target_type = distance;
    }
  }

  target_type = selected_target_type;
  if (target_type == TARGET_TYPE_COPY) {
    displayColor = YELLOW;
  } else if (target_type == TARGET_TYPE_MOVE) {
    displayColor = GREEN;
  } else {
    displayColor = OFF;
  }
}

void set_payload_for_face(byte* payload, byte f) {
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

void fwd_message_handler(byte id, byte src_face, byte dst_face, byte* payload) {
  if (src_face == FACE_COUNT) {
    // We are the source of the coordinate system. Set payload using the real
    // face number.
    set_payload_for_face(payload, dst_face);

    target_type = TARGET_TYPE_NONE;

    return;
  }

  // The input face is the face opposite to the face the message was sent to.
  byte input_face = ((payload[3] + 3) % 6);

  // The output face is the normalized face in relation to the input face.
  byte output_face = ((dst_face - src_face) + input_face + 6) % 6;

  // Set payload using the normalized output face.
  set_payload_for_face(payload, output_face);
}

void fwd_reply_handler(byte id, byte* payload) {
  if (target_type != TARGET_TYPE_NONE) {
    // Just indicate we have a target for now.
    payload[0] = 1;
  }
}

message::Message find_targets;

void setup() {
  message::Set(find_targets, MESSAGE_FIND_TARGETS, nullptr, false);
  message::manager::Set(rcv_message_handler, fwd_message_handler, nullptr,
                        fwd_reply_handler);
}

byte result[MESSAGE_PAYLOAD_BYTES];

void loop() {
  if (buttonSingleClicked()) {
    if (message::manager::Send(find_targets)) {
      displayColor = dim(BLUE, 127);
    } else {
      displayColor = RED;
    }
  }

  if (message::manager::Process(result)) {
    if (result[0] == 1) {
      // We have targets.
      displayColor = BLUE;
    } else {
      // No targets.
      displayColor = RED;
    }
  }

  setColor(displayColor);
}
