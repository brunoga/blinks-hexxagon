#include "game_message.h"

#include <string.h>

#include "blink_state.h"
#include "game_map.h"
#include "game_player.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-broadcast/message.h"
#include "src/blinks-orientation/orientation.h"
#include "src/blinks-position/position.h"

#define MESSAGE_STATE_SEND_MESSAGE 0
#define MESSAGE_STATE_WAIT_FOR_RESULT 1

// Coordinates stored in the find targets payload.
#define PAYLOAD_X 0
#define PAYLOAD_Y 1

namespace game {

namespace message {

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
    case MESSAGE_SELECT_ORIGIN:
      game::map::SetMoveOrigin((int8_t)payload[0], (int8_t)payload[1]);

      if (blink::state::GetPlayer() != 0) break;

      if (position::Distance(position::Coordinates{(int8_t)payload[0],
                                                   (int8_t)payload[1]}) <= 2) {
        blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_TARGET);
      }
      break;
    case MESSAGE_SELECT_TARGET:
      game::map::SetMoveTarget((int8_t)payload[0], (int8_t)payload[1]);
      break;
    case MESSAGE_FLASH:
      blink::state::StartColorOverride();
      break;
  }
}

static byte fwd_message_handler(byte message_id, byte src_face, byte dst_face,
                                byte* payload) {
  (void)src_face;
  (void)payload;

  byte len = 2;

  switch (message_id) {
    case MESSAGE_GAME_STATE_CHANGE:
      len = 1;
      break;
    case MESSAGE_EXTERNAL_PROPAGATE_COORDINATES:
      payload[0] = orientation::RelativeLocalFace(dst_face);
      len = 4;
      break;
  }

  return len;
}

static bool sendMessage(byte message_id, const byte* payload,
                        byte payload_size) {
  broadcast::Message message;
  message.header.id = message_id;

  if (payload != nullptr) {
    memcpy(message.payload, payload, payload_size);
  }

  return broadcast::manager::Send(&message);
}

void Setup() {
  broadcast::manager::Setup(rcv_message_handler, fwd_message_handler);
}

void __attribute__((noinline)) Process() { broadcast::manager::Process(); }

bool SendGameStateChange(byte payload) {
  return sendMessage(MESSAGE_GAME_STATE_CHANGE, &payload, 1);
}

bool SendSelectOrigin(int8_t x, int8_t y) {
  byte payload[2] = {(byte)x, (byte)y};
  return sendMessage(MESSAGE_SELECT_ORIGIN, payload, 2);
}

bool SendSelectTarget(int8_t x, int8_t y) {
  byte payload[2] = {(byte)x, (byte)y};
  return sendMessage(MESSAGE_SELECT_TARGET, payload, 2);
}

bool SendExternalPropagateCoordinates(int8_t x, int8_t y, byte player) {
  byte payload[4] = {FACE_COUNT, (byte)x, (byte)y, player};
  return sendMessage(MESSAGE_EXTERNAL_PROPAGATE_COORDINATES, payload, 4);
}

bool SendFlash() {
  if (sendMessage(MESSAGE_FLASH, nullptr, 0)) {
    blink::state::StartColorOverride();

    return true;
  }

  return false;
}

}  // namespace message

}  // namespace game
