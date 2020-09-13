#include "render_animation.h"

// (255 * 3) + 200
#define RENDER_ANIMATION_EXPLOSION_MS 965

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;
static bool explosion_animation_started_ = false;

void Pulse(const Color& base_color, byte start, byte speed) {
  if (timer_.isExpired()) {
    reverse_ = !reverse_;
    timer_.set((255 - start) * speed);
  }

  byte base_brightness = timer_.getRemaining() / speed;

  byte brightness = reverse_ ? 255 - base_brightness : start + base_brightness;

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

bool Explosion(const Color& base_color) {
  if (timer_.isExpired()) {
    if (explosion_animation_started_) {
      explosion_animation_started_ = false;

      return true;
    } else {
      explosion_animation_started_ = true;
    }

    timer_.set(RENDER_ANIMATION_EXPLOSION_MS);
  }

  if (timer_.getRemaining() > 200) {
    setColor(lighten(base_color, 255 - ((timer_.getRemaining() - 200) / 3)));
  } else if (timer_.getRemaining() > 100) {
    setColor(WHITE);
  } else {
    setColor(OFF);
  }
  return false;
}

}  // namespace animation

}  // namespace render
