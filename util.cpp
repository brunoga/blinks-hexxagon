#include "util.h"

#include <blinklib.h>

#include "game_map.h"
#include "game_message.h"

namespace util {

bool NoSleepButtonSingleClicked() {
  return buttonSingleClicked() && !hasWoken();
}

bool CheckValidateStateAndReport(bool* valid) {
  return ((*valid = game::map::ValidState()) || game::message::SendFlash());
}

}  // namespace util
