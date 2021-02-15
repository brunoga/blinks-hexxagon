#include "blink_state_face.h"

#include "blink_state.h"
#include "game_state.h"

namespace blink {

namespace state {

namespace face {

namespace handler {

static Value previous_value_[FACE_COUNT];

static byte previously_connected_faces_;
static byte wants_connection_faces_;
static byte wants_disconnection_faces_;

static bool reset_state_;

static byte map_requested_face_;
static bool enemy_neighbor_;

static void __attribute__((noinline)) reset_game() {
  if (game::state::Get() == GAME_STATE_IDLE) return;

  FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }

  blink::state::StartColorOverride();

  game::state::Set(GAME_STATE_IDLE, true);
}

void ProcessTop() {
  map_requested_face_ = FACE_COUNT;
  enemy_neighbor_ = false;

  byte currently_connected_faces = 0;

  byte game_state = game::state::Get();

  FOREACH_FACE(face) {
    byte face_mask = (1 << face);

    Value value = {.as_byte = getLastValueReceivedOnFace(face)};

    if (!value.map_requested) {
      if (isValueReceivedOnFaceExpired(face)) {
        if (previously_connected_faces_ & face_mask) {
          // Face just disconnected.
          if (game_state > GAME_STATE_SETUP_SELECT_PLAYERS &&
              game_state < GAME_STATE_PLAY) {
            // Blink removed while mapping. Reset game.
            ResetGame();
            return;
          }

          if (!(wants_disconnection_faces_ & face_mask)) {
            // And it was not one we expected to be disconnected.
            wants_connection_faces_ |= face_mask;
          } else {
            // We wanted it to be disconnected. All good.
            wants_disconnection_faces_ &= ~face_mask;
          }
        }
        continue;
      } else {
        if (!(previously_connected_faces_ & face_mask)) {
          // Face just connected.
          if (!(wants_connection_faces_ & face_mask)) {
            // And it is not one we expected to be connecting to us.
            wants_disconnection_faces_ |= face_mask;
          } else {
            // We wanted it to be connected. All good.
            wants_connection_faces_ &= ~face_mask;
          }
        }
      }

      currently_connected_faces |= face_mask;
    }

    if (value.reset_state != previous_value_[face].reset_state) {
      reset_state_ = value.reset_state;
      reset_game();
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

  // TODO(BGA): Find a better way to do this.
  if (game::state::Get() < GAME_STATE_PLAY ||
      game::state::Get() >= GAME_STATE_END) {
    wants_connection_faces_ = 0;
    wants_disconnection_faces_ = 0;
  }
}

void ProcessBottom() {
  Value output_value = {0, blink::state::GetColorOverride(), reset_state_,
                        false, blink::state::GetPlayer()};
  setValueSentOnAllFaces(output_value.as_byte);
}

bool EnemyNeighbor() { return enemy_neighbor_; }

byte MapRequestedFace() { return map_requested_face_; }

bool FaceOk(byte face) {
  return !((wants_connection_faces_ | wants_disconnection_faces_) &
           (1 << face));
}

void ResetGame() {
  reset_state_ = !reset_state_;

  reset_game();
}

}  // namespace handler

}  // namespace face

}  // namespace state

}  // namespace blink
