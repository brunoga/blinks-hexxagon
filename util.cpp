#include "util.h"

#include <blinklib.h>

#include "blink_state.h"

namespace util {

bool NoSleepButtonSingleClicked() {
  return buttonSingleClicked() && !hasWoken();
}

bool SearchNeighborType(byte neighbor_type, byte* source_face) {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      blink::state::FaceValue face_value;
      face_value.as_byte = getLastValueReceivedOnFace(f);

      *source_face = f;

      switch (neighbor_type) {
        case NEIGHBOR_TYPE_TARGET:
          if (face_value.target) {
            return true;
          }
          break;
        case NEIGHBOR_TYPE_ENEMY:
          if ((face_value.player != 0) &&
              (face_value.player != blink::state::GetPlayer())) {
            return true;
          }
          break;
        case NEIGHBOR_TYPE_SELF_DESTRUCT:
          if (face_value.self_destruct) {
            return true;
          }
          break;
      }
    }
  }

  return false;
}

}  // namespace util
