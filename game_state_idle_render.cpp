#include "game_state_idle_render.h"

#include "blink_state.h"

namespace game {

namespace state {

namespace idle {

static Timer pulse_timer_;
static bool reverse_ = true;

void Render() {
  if (pulse_timer_.isExpired()) {
    reverse_ = !reverse_;
    pulse_timer_.add(128 * 10);
  }

  byte base_brightness = pulse_timer_.getRemaining() / 10;

  byte brightness = reverse_ ? 255 - base_brightness : base_brightness + 127;

  setColor(dim(ORANGE, brightness));
}

}  // namespace idle

}  // namespace state

}  // namespace game