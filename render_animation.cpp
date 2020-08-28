#include "render_animation.h"

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;

void Pulsate(const Color& base_color) {
  if (timer_.isExpired()) {
    reverse_ = !reverse_;
    timer_.set(128 * 10);
  }

  byte base_brightness = timer_.getRemaining() / 10;

  byte brightness = reverse_ ? 255 - base_brightness : base_brightness + 127;

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