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
static byte previous_map_requested_face_ = FACE_COUNT;

bool Process(const blink::state::face::ValueHandler& face_value_handler) {
  byte current_map_requested_face = face_value_handler.MapRequestedFace();
  if (current_map_requested_face != previous_map_requested_face_) {
    resetPendingDatagramOnFace(previous_map_requested_face_);
    index_ = 0;
    previous_map_requested_face_ = current_map_requested_face;
  }

  if ((current_map_requested_face == FACE_COUNT) || Uploaded() ||
      (game::state::Get() != GAME_STATE_PLAY_SELECT_ORIGIN)) {
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
      if (sendDatagramOnFace(payload, GAME_MAP_UPLOAD_METADATA_SIZE,
                             current_map_requested_face)) {
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
      if (sendDatagramOnFace(&(map_data[index_]), delta * 2,
                             current_map_requested_face)) {
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
