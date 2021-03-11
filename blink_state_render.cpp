#include "blink_state_render.h"

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_player.h"
#include "game_state.h"

// (255 * 3) + 200
#define BLINK_STATE_RENDER_EXPLOSION_MS 1020
#define BLINK_STATE_RENDER_BLINK_MS 200

namespace blink {

namespace state {

namespace render {

static Timer pulse_timer_;
static Timer explosion_timer_;
static Timer blink_timer_;

static bool reverse_ = true;
static bool animation_started_;

static bool blink_on_;

static bool reset_timer_if_expired(Timer* timer, word ms) {
  if (timer->isExpired()) {
    timer->set(ms);

    return true;
  }

  return false;
}

static byte compute_pulse_dim(byte start, byte slowdown) {
  if (pulse_timer_.isExpired()) {
    pulse_timer_.set((255 - start) * slowdown);
    reverse_ = !reverse_;
  }

  byte base_brightness = pulse_timer_.getRemaining() / slowdown;

  return reverse_ ? 255 - base_brightness : start + base_brightness;
}

void ResetPulseTimer() {
  pulse_timer_.set(0);
  reverse_ = false;
}

void Pulse(byte start, byte slowdown) {
  Player(compute_pulse_dim(start, slowdown));
}

void Spinner(const Color& spinner_color, byte slowdown) {
  setColorOnFace(spinner_color, (millis() / slowdown) % FACE_COUNT);
}

bool Explosion(Color base_color) {
  if (reset_timer_if_expired(&explosion_timer_,
                             BLINK_STATE_RENDER_EXPLOSION_MS)) {
    if (animation_started_) {
      animation_started_ = false;

      return true;
    } else {
      animation_started_ = true;
    }
  }

  setColor(lighten(base_color, 255 - (explosion_timer_.getRemaining() / 4)));

  return false;
}

void Player(byte dim_level) {
  if (reset_timer_if_expired(&blink_timer_, BLINK_STATE_RENDER_BLINK_MS)) {
    blink_on_ = !blink_on_;
  }

  // It is actually cheaper to call setColorOnFace() multiple times below than
  // to use a variable and call it only once.
  FOREACH_FACE(face) {
    if (!blink::state::face::handler::FaceOk(face)) {
      if (blink_on_) {
        setColorOnFace(WHITE, face);
      } else {
        setColorOnFace(OFF, face);
      }
    } else if (game::player::GetLitFace(blink::state::GetPlayer(), face)) {
      setColorOnFace(
          dim(game::player::GetColor(blink::state::GetPlayer()), dim_level),
          face);
    } else {
      setColorOnFace(OFF, face);
    }
  }
}

}  // namespace render

}  // namespace state

}  // namespace blink
