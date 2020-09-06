#include "render_animation.h"

#define RENDER_ANIMATION_EXPLOSION_MS 800

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;
static bool explosion_animation_started_ = false;

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

static byte __attribute__((noinline)) compute_color_component(byte start) {
  return start + (((255 - start) *
                   (((RENDER_ANIMATION_EXPLOSION_MS - timer_.getRemaining()) *
                     RENDER_ANIMATION_EXPLOSION_MS) /
                    RENDER_ANIMATION_EXPLOSION_MS)) /
                  RENDER_ANIMATION_EXPLOSION_MS);
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

  byte r = compute_color_component(base_color.r);
  byte g = compute_color_component(base_color.g);
  byte b = compute_color_component(base_color.b);

  setColor(makeColorRGB(r, g, b));

  return false;
}

}  // namespace animation

}  // namespace render
