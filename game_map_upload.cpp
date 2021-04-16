#include "game_map_upload.h"

#include <blinklib.h>
#include <string.h>

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-mapping/mapping.h"

namespace game {

namespace map {

namespace upload {

static byte previous_ai_face_ = FACE_COUNT;

static mapping::Iterator iterator_;
static bool map_uploaded_;

bool Process() {
  byte current_ai_face = blink::state::face::handler::AIFace();
  if (current_ai_face != previous_ai_face_) {
    if (previous_ai_face_ != FACE_COUNT) {
      resetPendingDatagramOnFace(previous_ai_face_);
    }

    Reset();

    previous_ai_face_ = current_ai_face;
  }

  if ((current_ai_face == FACE_COUNT) || map_uploaded_ ||
      (game::state::Get() != GAME_STATE_PLAY_SELECT_ORIGIN)) {
    return false;
  }

  int8_t x;
  int8_t y;
  byte value = mapping::GetNextValidPosition(&iterator_, &x, &y);

  if (value == MAPPING_POSITION_EMPTY) {
    map_uploaded_ = true;

    // Force sending a game state update message so the AI has up-to-date data.
    game::state::Set(GAME_STATE_PLAY, true);
    game::state::Set(GAME_STATE_PLAY_SELECT_ORIGIN);

    return false;
  }

  byte payload[4] = {MESSAGE_MAP_UPLOAD, (byte)x, (byte)y, value};

  if (!sendDatagramOnFace(payload, 4, current_ai_face)) {
    // TODO(bga): Need to resend the same position. There is currently no way to
    // do that.
  }

  return true;
}

void __attribute__((noinline)) Reset() {
  // Doing this is enough to reset the iterator.
  iterator_.initialized = false;

  map_uploaded_ = false;
}

}  // namespace upload

}  // namespace map

}  // namespace game
