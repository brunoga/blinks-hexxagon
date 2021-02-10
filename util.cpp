#include "util.h"

#include <blinklib.h>

#include "blink_state.h"
#include "game_map.h"

namespace util {

bool NoSleepButtonSingleClicked() {
  return buttonSingleClicked() && !hasWoken();
}

bool CheckValidateStateAndReport() {
  if (!game::map::ValidState()) {
    blink::state::StartColorOverride();

    return false;
  }

  return true;
}

}  // namespace util
