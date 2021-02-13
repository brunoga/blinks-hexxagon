#include "blink_state_face.h"

#include "blink_state.h"
#include "game_state.h"

namespace blink {

namespace state {

namespace face {

ValueHandler::ValueHandler()
    : map_requested_face_(FACE_COUNT), enemy_neighbor_(false) {
  byte currently_connected_faces = 0;

  FOREACH_FACE(face) {
    if (isValueReceivedOnFaceExpired(face)) {
      if (previously_connected_faces_ & (1 << face)) {
        ResetGame();
      }
      continue;
    }

    currently_connected_faces |= (1 << face);

    Value value = {.as_byte = getLastValueReceivedOnFace(face)};

    if (value.reset_state != previous_value_[face].reset_state) {
      reset_state_ = value.reset_state;
      InternalResetGame();
    }

    if (value.color_override != previous_value_[face].color_override &&
        value.color_override) {
      blink::state::StartColorOverride();
    }

    if (value.map_requested) {
      map_requested_face_ = face;
    }

    if (value.player != 0 && value.player != blink::state::GetPlayer()) {
      enemy_neighbor_ = true;
    }

    previous_value_[face] = value;
  }

  previously_connected_faces_ = currently_connected_faces;
}

ValueHandler::~ValueHandler() {
  Value output_value = {0, blink::state::GetColorOverride(), reset_state_,
                        false, blink::state::GetPlayer()};
  setValueSentOnAllFaces(output_value.as_byte);
}

bool ValueHandler::EnemyNeighbor() const { return enemy_neighbor_; }

byte ValueHandler::MapRequestedFace() const { return map_requested_face_; }

void ValueHandler::ResetGame() {
  reset_state_ = !reset_state_;

  InternalResetGame();
}

void __attribute__((noinline)) ValueHandler::InternalResetGame() {
  if (game::state::Get() == GAME_STATE_IDLE) return;

  FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }

  blink::state::StartColorOverride();

  game::state::Set(GAME_STATE_IDLE, true);
}

Value ValueHandler::previous_value_[FACE_COUNT] = {
    {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
};

byte ValueHandler::previously_connected_faces_ = 0;

bool ValueHandler::reset_state_ = false;

}  // namespace face

}  // namespace state

}  // namespace blink
