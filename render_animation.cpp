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

#define RENDER_ANIMATION_TAKEOVER_MS 1000

static byte compute_color_component(byte start,
                                    const millis_t& remaining_time_ms) {
  return start + (((255 - start) / RENDER_ANIMATION_TAKEOVER_MS) *
                  (RENDER_ANIMATION_TAKEOVER_MS - remaining_time_ms));
}

bool Takeover(const Color& start, const Color& end) {
  if (timer_.isExpired()) {
    if (takeover_animation_started_) {
      takeover_animation_started_ = false;

      return true;
    } else {
      takeover_animation_started_ = true;
    }

    timer_.set(RENDER_ANIMATION_TAKEOVER_MS);
  }

  millis_t remaining = timer_.getRemaining();

  byte r = compute_color_component(start.r, remaining);
  byte g = compute_color_component(start.g, remaining);
  byte b = compute_color_component(start.b, remaining);

  setColor(makeColorRGB(r, g, b));

  return false;
}

}  // namespace animation

}  // namespace render
