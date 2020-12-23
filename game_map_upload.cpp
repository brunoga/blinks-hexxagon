#include "game_map_upload.h"

#include <blinklib.h>

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_play.h"

#define GAME_MAP_UPLOAD_STATE_SEND_METADATA 0
#define GAME_MAP_UPLOAD_STATE_UPLOAD 1

#define GAME_MAP_UPLOAD_METADATA_SIZE 3

// 2 byte entries (4 bytes total). Chosen to be smaller than the maximum size of
// datagram we use otherwise.
#define GAME_MAP_UPLOAD_MAX_CHUNK_SIZE 2

namespace game {

namespace map {

namespace upload {

static byte index_;
static byte state_;

static void update_map_requested_face() {
  byte map_requested_face = blink::state::GetMapRequestedFace();

  // Check if we are currently connected.
  if (map_requested_face != FACE_COUNT) {
    // We are, did we get disconnected?
    if (isValueReceivedOnFaceExpired(map_requested_face)) {
      Reset();

      // Make sure we will not leave data hanging if disconnected in the middle
      // of a transfer.
      resetPendingDatagramOnFace(map_requested_face);

      blink::state::SetMapRequestedFace(FACE_COUNT);
    }

    return;
  }

  FOREACH_FACE(face) {
    // Parse face value so we can check for a map being requested.
    blink::state::FaceValue face_value = {.as_byte =
                                              getLastValueReceivedOnFace(face)};

    if (!isValueReceivedOnFaceExpired(face) && face_value.map_requested) {
      // A map is requested in this face.
      blink::state::SetMapRequestedFace(face);

      return;
    }
  }
}

bool Process() {
  update_map_requested_face();

  byte face = blink::state::GetMapRequestedFace();

  if ((face == FACE_COUNT) || Uploaded() ||
      (game::state::Get() != GAME_STATE_PLAY) ||
      (game::state::GetSpecific() != GAME_STATE_PLAY_SELECT_ORIGIN)) {
    // Only send a map when we are sure we have one.
    return false;
  }

  const game::map::Data* map_data = game::map::Get();
  byte map_size = game::map::GetSize();

  switch (state_) {
    case GAME_MAP_UPLOAD_STATE_SEND_METADATA: {
      // Upload just started. Send map metadata.
      byte payload[GAME_MAP_UPLOAD_METADATA_SIZE] = {
          MESSAGE_MAP_UPLOAD, map_size, game::state::GetData()};
      if (sendDatagramOnFace(payload, GAME_MAP_UPLOAD_METADATA_SIZE, face)) {
        // Size sent. Switch to actual map upload.
        state_ = GAME_MAP_UPLOAD_STATE_UPLOAD;
      }
      break;
    }
    case GAME_MAP_UPLOAD_STATE_UPLOAD:
      // Now upload the actual map in chunks of
      // GAME_MAP_UPLOAD_MAX_CHUNK_SIZE.
      byte remaining = map_size - index_;
      byte delta = remaining > GAME_MAP_UPLOAD_MAX_CHUNK_SIZE
                       ? GAME_MAP_UPLOAD_MAX_CHUNK_SIZE
                       : remaining;
      if (sendDatagramOnFace(&(map_data[index_]), delta * 2, face)) {
        // Chunk sent. Increase the map upload index.
        index_ += delta;
      }
      break;
  }

  return true;
}

bool Uploaded() {
  byte map_size = game::map::GetSize();

  return ((map_size > 0) && (index_ == map_size));
}

void Reset() {
  state_ = GAME_MAP_UPLOAD_STATE_SEND_METADATA;
  index_ = 0;
}

}  // namespace upload

}  // namespace map

}  // namespace game
