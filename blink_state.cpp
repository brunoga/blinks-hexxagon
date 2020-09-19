#include "blink_state.h"

#include <blinklib.h>

#include "game_state.h"
#include "game_state_end_render.h"
#include "game_state_idle_render.h"
#include "game_state_play_render.h"
#include "game_state_setup_render.h"

namespace blink {

namespace state {

struct BlinkState {
  bool origin;
  bool target;
  bool animating;
  byte player;
  byte target_type;
};
static BlinkState state_;

static Timer color_override_timer_;

static void* animating_param_;
static bool (*animating_function_)(void* param);

void SetOrigin(bool origin) { state_.origin = origin; }
bool __attribute__((noinline)) GetOrigin() { return state_.origin; }

void SetTarget(bool target) { state_.target = target; }
bool __attribute__((noinline)) GetTarget() { return state_.target; }

void SetTargetType(byte target_type) { state_.target_type = target_type; }
byte GetTargetType() { return state_.target_type; }

void __attribute__((noinline)) SetPlayer(byte player) {
  state_.player = player;
}
byte GetPlayer() { return state_.player; }

void SetAnimating(bool animating) { state_.animating = animating; }
bool GetAnimating() { return state_.animating; }

void SetAnimatingParam(void* animating_param) {
  animating_param_ = animating_param;
}
void SetAnimatingFunction(bool (*animating_function)(void* param)) {
  animating_function_ = animating_function;
}

bool RunAnimatingFunction() { return animating_function_(animating_param_); }

void __attribute__((noinline)) StartColorOverride() {
  color_override_timer_.set(200);
}

bool GetColorOverride() { return !color_override_timer_.isExpired(); }

void __attribute__((noinline)) Reset() {
  state_.origin = false;
  state_.target = false;
  state_.animating = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
}

void __attribute__((noinline)) Render(byte game_state) {
  // "Render" our face value.
  FaceValue face_value;

  face_value.target = GetTarget();
  face_value.player = GetPlayer();

  setValueSentOnAllFaces(face_value.as_byte);

  if (blink::state::GetColorOverride()) {
    setColor(WHITE);

    return;
  }

  // Now render the state specific colors/animations.
  switch (game_state) {
    case GAME_STATE_IDLE:
      game::state::idle::Render();
      break;
    case GAME_STATE_SETUP:
      game::state::setup::Render();
      break;
    case GAME_STATE_PLAY:
      game::state::play::Render();
      break;
    case GAME_STATE_END:
      game::state::end::Render();
      break;
  }
}

}  // namespace state

}  // namespace blink
