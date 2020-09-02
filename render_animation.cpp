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

static bool takeover_animation_started_ = false;

bool Takeover(const Color& start, const Color& end) {
  if (timer_.isExpired()) {
    if (takeover_animation_started_) {
      takeover_animation_started_ = false;

      return true;
    } else {
      takeover_animation_started_ = true;
    }

    timer_.set(1000);
  }

  // TODO(bga): Actually animate this. It should look like an explosion going
  // from the start color to the end one passing through white. The current
  // implementation is just for testing.
  if (timer_.getRemaining() < 450) {
    setColor(start);
  } else if (timer_.getRemaining() < 550) {
    setColor(WHITE);
  } else {
    setColor(end);
  }

  return false;
}

}  // namespace animation

}  // namespace render