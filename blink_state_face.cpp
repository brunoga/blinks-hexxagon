#include "blink_state_face.h"

#include "blink_state.h"
#include "game_state.h"

namespace blink {

namespace state {

namespace face {

ValueHandler::ValueHandler() {
  // Cache the previously connected faces and zero-out the currently connected
  // ones.
  previously_connected_faces_ = currently_connected_faces_;
  currently_connected_faces_ = 0;

  map_requested_face_ = FACE_COUNT;

  FOREACH_FACE(face) {
    Value value = {.as_byte = getLastValueReceivedOnFace(face)};

    if (!isValueReceivedOnFaceExpired(face)) {
      currently_connected_faces_ |= 1 << face;
    } else {
      value.map_requested = false;
      value.player = 0;
    }

    if (value.reset_state != previous_value_[face].reset_state) {
      InternalResetGame(value.reset_state);
    }

    if (value.color_override != previous_value_[face].color_override &&
        value.color_override) {
      blink::state::StartColorOverride();
    }

    if (value.map_requested) {
      map_requested_face_ = face;
    }

    previous_value_[face] = value;
  }
}

ValueHandler::~ValueHandler() {
  Value output_value = {0, blink::state::GetColorOverride(), reset_state_,
                        false, blink::state::GetPlayer()};
  setValueSentOnAllFaces(output_value.as_byte);
}

byte ValueHandler::GetPlayerAtFace(byte face) const {
  return previous_value_[face].player;
}

bool ValueHandler::FaceConnected(byte face) const {
  // Face was not connected before and is connected now.
  return ((previously_connected_faces_ & (1 << face)) == 0) &&
         ((currently_connected_faces_ & (1 << face)) != 0);
}

bool ValueHandler::FaceDisconnected(byte face) const {
  // Face was connected before and is not connected now.
  return ((previously_connected_faces_ & (1 << face)) != 0) &&
         ((currently_connected_faces_ & (1 << face)) == 0);
}

bool ValueHandler::MapRequestedFace() const { return map_requested_face_; }

void __attribute__((noinline)) ValueHandler::ResetGame() {
  InternalResetGame(!reset_state_);
}

void __attribute__((noinline))
ValueHandler::InternalResetGame(bool new_reset_state) {
  if (new_reset_state == reset_state_) return;

  FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }

  reset_state_ = new_reset_state;

  blink::state::StartColorOverride();

  game::state::Set(GAME_STATE_IDLE, true);
}

Value ValueHandler::previous_value_[FACE_COUNT] = {
    {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
};

byte ValueHandler::previously_connected_faces_ = 0;

byte ValueHandler::currently_connected_faces_ = 0;

}  // namespace face

}  // namespace state

}  // namespace blink
