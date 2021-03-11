#include "game_map_upload.h"

#include <blinklib.h>
#include <string.h>

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_play.h"

#define GAME_MAP_UPLOAD_STATE_SEND_METADATA 0
#define GAME_MAP_UPLOAD_STATE_UPLOAD 1

#define GAME_MAP_UPLOAD_METADATA_SIZE 2

// 2 byte entries (4 bytes total). Chosen to be smaller than the maximum size of
// datagram we use otherwise.
#define GAME_MAP_UPLOAD_MAX_CHUNK_SIZE 2

namespace game {

namespace map {

namespace upload {

static byte index_;
static byte state_;
static byte previous_ai_face_ = FACE_COUNT;

bool Process() {
  byte current_ai_face = blink::state::face::handler::AIFace();
  if (current_ai_face != previous_ai_face_) {
    if (previous_ai_face_ != FACE_COUNT) {
      resetPendingDatagramOnFace(previous_ai_face_);
    }

    Reset();

    previous_ai_face_ = current_ai_face;
  }

  byte map_size = game::map::GetSize();

  if ((current_ai_face == FACE_COUNT) || (index_ == map_size) ||
      (game::state::Get() != GAME_STATE_PLAY_SELECT_ORIGIN)) {
    return false;
  }

  const game::map::Data* map_data = game::map::Get();

  byte payload[5];
  payload[0] = MESSAGE_MAP_UPLOAD;

  switch (state_) {
    case GAME_MAP_UPLOAD_STATE_SEND_METADATA: {
      // Upload just started. Send map metadata.
      payload[1] = map_size;

      if (sendDatagramOnFace(payload, GAME_MAP_UPLOAD_METADATA_SIZE,
                             current_ai_face)) {
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
      memcpy(&payload[1], &(map_data[index_]), (delta * 2));
      if (sendDatagramOnFace(payload, (delta * 2) + 1, current_ai_face)) {
        // Chunk sent. Increase the map upload index.
        index_ += delta;
      }

      if (index_ == map_size) {
        // Force sending a game state update message so the AI has up-to-date
        // data.
        game::state::Set(GAME_STATE_PLAY, true);
        game::state::Set(GAME_STATE_PLAY_SELECT_ORIGIN);
      }

      break;
  }

  return true;
}

void Reset() {
  index_ = 0;
  state_ = GAME_MAP_UPLOAD_STATE_SEND_METADATA;
}

}  // namespace upload

}  // namespace map

}  // namespace game
