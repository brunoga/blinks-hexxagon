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
  bool take_over;
#ifndef RENDER_ANIMATION_TAKE_OVER_DISABLE_LIGHTNING
  byte take_over_face;
#endif
  byte player;
  byte target_type;
};
static BlinkState state_;

static Timer color_override_timer_;

void SetOrigin(bool origin) { state_.origin = origin; }
bool __attribute__((noinline)) GetOrigin() { return state_.origin; }

void SetTarget(bool target) { state_.target = target; }
bool GetTarget() { return state_.target; }

void SetTargetType(byte target_type) { state_.target_type = target_type; }
byte GetTargetType() { return state_.target_type; }

void SetPlayer(byte player) { state_.player = player; }
byte GetPlayer() { return state_.player; }

void SetTakeOver(bool take_over) { state_.take_over = take_over; }
bool GetTakeOver() { return state_.take_over; }

#ifndef RENDER_ANIMATION_TAKE_OVER_DISABLE_LIGHTNING
void SetTakeOverFace(byte take_over_face) {
  state_.take_over_face = take_over_face;
}

byte GetTakeOverFace() { return state_.take_over_face; }

#endif

void __attribute__((noinline)) StartColorOverride() {
  color_override_timer_.set(200);
}

bool GetColorOverride() { return !color_override_timer_.isExpired(); }

void __attribute__((noinline)) Reset() {
  state_.origin = false;
  state_.target = false;
  state_.take_over = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
}

void Render(byte game_state) {
  // "Render" our face value.
  FaceValue face_value = {0, false, GetTarget(), GetPlayer()};

  setValueSentOnAllFaces(face_value.as_byte);

  if (GetColorOverride()) {
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
