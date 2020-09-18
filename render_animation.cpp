#include "render_animation.h"

// (255 * 3) + 200
#define RENDER_ANIMATION_EXPLOSION_MS 965

#define RENDER_ANIMATION_LIGHTNING_MS 200

namespace render {

namespace animation {

static Timer timer_;
static bool reverse_ = true;
static bool animation_started_ = false;

static byte end_;

void Pulse(const Color& base_color, byte start, byte slowdown) {
  if (timer_.isExpired()) {
    reverse_ = !reverse_;
    timer_.set((255 - start) * slowdown);
  }

  byte base_brightness = timer_.getRemaining() / slowdown;

  byte brightness = reverse_ ? 255 - base_brightness : start + base_brightness;

  setColor(dim(base_color, brightness));
}

void Spinner(const Color& spinner_color, byte num_faces, byte slowdown) {
  if (timer_.isExpired()) {
    timer_.set((FACE_COUNT * slowdown) - 1);
  }

  byte f = (FACE_COUNT - 1) - timer_.getRemaining() / slowdown;

  byte step = FACE_COUNT / num_faces;

  for (byte face = 0; face < FACE_COUNT; face += step) {
    setColorOnFace(spinner_color, (f + face) % FACE_COUNT);
  }
}

bool Explosion(const Color& base_color) {
  if (timer_.isExpired()) {
    if (animation_started_) {
      animation_started_ = false;

      return true;
    } else {
      animation_started_ = true;
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

bool Lightning(byte origin_face) {
  if (timer_.isExpired()) {
    if (animation_started_) {
      animation_started_ = false;

      return true;
    } else {
      animation_started_ = true;
    }

    timer_.set(RENDER_ANIMATION_LIGHTNING_MS);

    end_ = random(2);
  }

  setColorOnFace(WHITE, origin_face);

  if (timer_.getRemaining() < RENDER_ANIMATION_LIGHTNING_MS - 25) {
    byte destination_face = (origin_face + 2 + end_) % FACE_COUNT;

    setColorOnFace(WHITE, destination_face);
  }

  return false;
}

}  // namespace animation

}  // namespace render
