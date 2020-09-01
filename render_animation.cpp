#include "render_animation.h"

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;

void Pulse(const Color& base_color, byte start, byte speed) {
  if (timer_.isExpired()) {
    reverse_ = !reverse_;
    timer_.set(start * speed);
  }

  byte base_brightness = timer_.getRemaining() / speed;

  byte brightness =
      reverse_ ? 255 - base_brightness : 255 - (start - base_brightness);

  setColor(dim(base_color, brightness));
}

void WhiteSpinner(const Color& base_color) {
  if (timer_.isExpired()) {
    timer_.set((5 * 100) + 99);
  }

  byte f = timer_.getRemaining() / 100;

  setColor(base_color);
  setColorOnFace(WHITE, f);
}

}  // namespace animation

}  // namespace render