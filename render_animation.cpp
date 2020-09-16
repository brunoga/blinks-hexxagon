#include "render_animation.h"

// (255 * 3) + 200
#define RENDER_ANIMATION_EXPLOSION_MS 965

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;
static bool explosion_animation_started_ = false;

void Pulse(const Color& base_color, byte start, byte slowdown) {
  if (timer_.isExpired()) {
    reverse_ = !reverse_;
    timer_.set((255 - start) * slowdown);
  }

  byte base_brightness = timer_.getRemaining() / slowdown;

  byte brightness = reverse_ ? 255 - base_brightness : start + base_brightness;

  setColor(dim(base_color, brightness));
}

void Spinner(const Color& base_color, const Color& spinner_color,
             byte num_faces, byte slowdown) {
  if (timer_.isExpired()) {
    timer_.set((FACE_COUNT * slowdown) - 1);
  }

  byte f = (FACE_COUNT - 1) - timer_.getRemaining() / slowdown;

  setColor(base_color);

  byte step = FACE_COUNT / num_faces;

  for (byte face = 0; face < FACE_COUNT; face += step) {
    setColorOnFace(spinner_color, (f + face) % FACE_COUNT);
  }
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
