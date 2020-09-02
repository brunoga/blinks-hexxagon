#include "blink_state.h"

#include <blinklib.h>

#include "debug.h"
#include "game_state.h"
#include "game_state_end_render.h"
#include "game_state_idle_render.h"
#include "game_state_play_render.h"
#include "game_state_setup_render.h"

namespace blink {

namespace state {

struct BlinkState {
  bool origin : 1;
  bool target : 1;
  byte player : 3;
  byte target_type : 1;
  bool color_override : 1;
  byte unused : 1;
};
static BlinkState state_;

void SetOrigin(bool origin) { state_.origin = origin; }
bool GetOrigin() { return state_.origin; }

void SetTarget(bool target) { state_.target = target; }
bool GetTarget() { return state_.target; }

void SetTargetType(byte target_type) { state_.target_type = target_type; }
byte GetTargetType() { return state_.target_type; }

void SetPlayer(byte player) { state_.player = player; }
byte GetPlayer() { return state_.player; }

void SetColorOverride(bool color_override) {
  state_.color_override = color_override;
}
bool GetColorOverride() { return state_.color_override; }

void Reset() {
  state_.origin = false;
  state_.target = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
  state_.color_override = false;
}

void Render(byte game_state) {
  // "Render" our face value.
  FaceValue face_value;

  face_value.origin = GetOrigin();
  face_value.target = GetTarget();
  face_value.player = GetPlayer();

  // Make sure everything else is zeroed out.
  face_value.unused = 0;
  face_value.reserved = 0;

  setValueSentOnAllFaces(face_value.value);

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
