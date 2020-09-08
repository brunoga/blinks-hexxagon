#include "util.h"

#include <blinklib.h>

namespace util {

bool NoSleepButtonSingleClicked() {
  return buttonSingleClicked() && !hasWoken();
}

}  // namespace util
