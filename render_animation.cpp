#include "render_animation.h"

// (255 * 3) + 200
#define RENDER_ANIMATION_EXPLOSION_MS 965

namespace render {

namespace animation {

static Timer pulse_timer_;
static Timer spinner_timer_;
static Timer explosion_timer_;

static bool reverse_ = true;
static bool animation_started_;

static bool reset_timer_if_expired(Timer* timer, word ms) {
  if (timer->isExpired()) {
    timer->set(ms);

    return true;
  }

  return false;
}

void ResetPulseTimer() {
  pulse_timer_.set(0);
  reverse_ = false;
}

void Pulse(const Color& base_color, byte start, byte slowdown) {
  if (reset_timer_if_expired(&pulse_timer_, (255 - start) * slowdown)) {
    reverse_ = !reverse_;
  }

  byte base_brightness = pulse_timer_.getRemaining() / slowdown;

  byte brightness = reverse_ ? 255 - base_brightness : start + base_brightness;

  setColor(dim(base_color, brightness));
}

void Spinner(const Color& spinner_color, byte slowdown) {
  reset_timer_if_expired(&spinner_timer_, (FACE_COUNT * slowdown) - 1);

  byte f = (FACE_COUNT - 1) - spinner_timer_.getRemaining() / slowdown;

  setColorOnFace(spinner_color, f);
}

bool __attribute__((noinline)) Explosion(Color base_color) {
  if (reset_timer_if_expired(&explosion_timer_,
                             RENDER_ANIMATION_EXPLOSION_MS)) {
    if (animation_started_) {
      animation_started_ = false;

      return true;
    } else {
      animation_started_ = true;
    }
  }

  if (explosion_timer_.getRemaining() > 200) {
    setColor(lighten(base_color,
                     255 - ((explosion_timer_.getRemaining() - 200) / 3)));
  } else if (explosion_timer_.getRemaining() > 100) {
    setColor(WHITE);
  } else {
    setColor(OFF);
  }

  return false;
}

}  // namespace animation

}  // namespace render
