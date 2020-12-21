#include "render_animation.h"

// (255 * 3) + 200
#define RENDER_ANIMATION_EXPLOSION_MS 965

namespace render {

namespace animation {

static Timer timer_;

static bool reverse_ = true;
static bool animation_started_;

static bool reset_timer_if_expired(word ms) {
  if (timer_.isExpired()) {
    timer_.set(ms);

    return true;
  }

  return false;
}

void ResetTimer() {
  timer_.set(0);
  reverse_ = false;
}

void Pulse(const Color& base_color, byte start, byte slowdown) {
  if (reset_timer_if_expired((255 - start) * slowdown)) {
    reverse_ = !reverse_;
  }

  byte base_brightness = timer_.getRemaining() / slowdown;

  byte brightness = reverse_ ? 255 - base_brightness : start + base_brightness;

  setColor(dim(base_color, brightness));
}

void Spinner(const Color& spinner_color, byte slowdown) {
  reset_timer_if_expired((FACE_COUNT * slowdown) - 1);

  byte f = (FACE_COUNT - 1) - timer_.getRemaining() / slowdown;

  if (f >= FACE_COUNT) {
    // When tyransitioning from the pulse animation to the spinner one, we might
    // have a remaining value in our timer that it is bigger thna we expected so
    // we need to add this check here to prevent writting to arbitrary memory.
    return;
  }

  setColorOnFace(spinner_color, f);
}

bool Explosion(Color base_color) {
  if (reset_timer_if_expired(RENDER_ANIMATION_EXPLOSION_MS)) {
    if (animation_started_) {
      animation_started_ = false;

      return true;
    } else {
      animation_started_ = true;
    }
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
