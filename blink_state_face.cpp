#include "blink_state_face.h"

#include "blink_state.h"
#include "game_state.h"

#define BLINK_STATE_FACE_WANTS_CONNECTED 0
#define BLINK_STATE_FACE_CONNECTED 1
#define BLINK_STATE_FACE_WANTS_DISCONNECTED 2
#define BLINK_STATE_FACE_DISCONNECTED 3

namespace blink {

namespace state {

namespace face {

namespace handler {

static Value previous_value_[FACE_COUNT];

static byte previously_connected_faces_;
static byte wants_connection_faces_;
static byte wants_disconnection_faces_;

static byte ai_face_;
static bool enemy_neighbor_;

static bool record_connection_state(bool check1, bool check2, byte face_mask,
                                    byte* mask1, byte* mask2) {
  if (check1) {
    // It just disconnected.
    if (check2) {
      // It is not one we expected to be disconnected.
      *mask1 |= face_mask;
      return true;
    } else {
      // We wanted it to be disconnected. All good.
      *mask2 &= ~face_mask;
    }
  }

  return false;
}

static byte check_face_connection(byte face_mask, bool expired) {
  bool previously_connected_face = previously_connected_faces_ & face_mask;
  bool wants_disconnection_face = wants_disconnection_faces_ & face_mask;
  bool wants_connection_face = wants_connection_faces_ & face_mask;
  bool track_connection = game::state::Get() >= GAME_STATE_SETUP_MAP &&
                          game::state::Get() < GAME_STATE_END;

  if (expired) {
    // Face is not connected.
    if (track_connection) {
      if (record_connection_state(
              previously_connected_face, !wants_disconnection_face, face_mask,
              &wants_connection_faces_, &wants_disconnection_faces_)) {
        return BLINK_STATE_FACE_WANTS_CONNECTED;
      }
    }
    return BLINK_STATE_FACE_DISCONNECTED;
  } else {
    // Face is connected.
    if (track_connection) {
      if (record_connection_state(
              !previously_connected_face, !wants_connection_face, face_mask,
              &wants_disconnection_faces_, &wants_connection_faces_)) {
        return BLINK_STATE_FACE_WANTS_DISCONNECTED;
      }
    }
    return BLINK_STATE_FACE_CONNECTED;
  }
}

void ProcessTop() {
  ai_face_ = FACE_COUNT;
  enemy_neighbor_ = false;

  byte currently_connected_faces = 0;

  FOREACH_FACE(face) {
    bool expired = isValueReceivedOnFaceExpired(face);

    Value value = {.as_byte = getLastValueReceivedOnFace(face)};

    if (value.ai && !value.hexxagon) {
      if (!expired && value.map_requested) {
        // The Blink connected to this face looks like an AI and is requesting
        // the map.
        ai_face_ = face;
      }

      continue;
    }

    byte face_mask = (1 << face);

    switch (check_face_connection(face_mask, expired)) {
      case BLINK_STATE_FACE_CONNECTED:
        currently_connected_faces |= face_mask;
        break;
      case BLINK_STATE_FACE_WANTS_DISCONNECTED:
        currently_connected_faces |= face_mask;
        // FALLTHROUGH
      case BLINK_STATE_FACE_DISCONNECTED:
        continue;
      case BLINK_STATE_FACE_WANTS_CONNECTED:
        if (game::state::Get() == GAME_STATE_SETUP_MAP) {
          // Blink removed while mapping. Reset game.
          ResetGame();
        }
        continue;
    }

    if (value.reset_state != previous_value_[face].reset_state &&
        value.reset_state) {
      // Connected Blink changed state to reset, so we also reset.
      ResetGame();
    } else if (value.color_override != previous_value_[face].color_override &&
               value.color_override) {
      // Color override is on and it was not because the game was reset.
      blink::state::StartColorOverride();
    }

    if (value.player != 0 && value.player != blink::state::GetPlayer()) {
      enemy_neighbor_ = true;
    }

    previous_value_[face] = value;
  }

  previously_connected_faces_ = currently_connected_faces;
}

void ProcessBottom() {
  Value output_value = {/*map_requested=*/false,
                        /*hexxagon=*/true,
                        /*color_override=*/blink::state::GetColorOverride(),
                        /*reset_state=*/game::state::Get() == GAME_STATE_IDLE,
                        /*ai=*/false,
                        /*player=*/blink::state::GetPlayer()};
  setValueSentOnAllFaces(output_value.as_byte);
}

bool EnemyNeighbor() { return enemy_neighbor_; }

byte AIFace() { return ai_face_; }

bool FaceOk(byte face) {
  return !((wants_connection_faces_ | wants_disconnection_faces_) &
           (1 << face));
}

void ResetGame() {
  if (game::state::Get() == GAME_STATE_IDLE) return;

  // wants_connection_faces_ = 0;
  // wants_disconnection_faces_ = 0;

  FOREACH_FACE(face) { resetPendingDatagramOnFace(face); }

  blink::state::StartColorOverride();

  game::state::Set(GAME_STATE_IDLE, true);
}

void Reset() {
  previously_connected_faces_ = 0;
  wants_connection_faces_ = 0;
  wants_disconnection_faces_ = 0;
}

}  // namespace handler

}  // namespace face

}  // namespace state

}  // namespace blink
