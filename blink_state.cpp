#include "blink_state.h"

#include <blinklib.h>

#include "debug.h"
#include "game_state.h"
#include "game_state_idle_render.h"
#include "game_state_play_render.h"
#include "game_state_setup_render.h"

namespace blink {

namespace state {

struct BlinkState {
  byte type : 1;
  bool origin : 1;
  bool target : 1;
  byte player : 2;
  byte target_type : 2;
  bool arbitrator : 1;
};
static BlinkState state_;

void SetType(byte type) { state_.type = type; }
byte GetType() { return state_.type; }

void SetOrigin(bool origin) { state_.origin = origin; }
bool GetOrigin() { return state_.origin; }

void SetTarget(bool target) { state_.target = target; }
bool GetTarget() { return state_.target; }

void SetTargetType(byte target_type) { state_.target_type = target_type; }
byte GetTargetType() { return state_.target_type; }

void SetPlayer(byte player) { state_.player = player - 1; }
byte GetPlayer() {
  return state_.type == BLINK_STATE_TYPE_PLAYER ? state_.player + 1 : 0;
}

void SetArbitrator(bool arbitrator) { state_.arbitrator = arbitrator; }
bool GetArbitrator() { return state_.arbitrator; }

void Reset() {
  state_.type = BLINK_STATE_TYPE_EMPTY;
  state_.origin = false;
  state_.target = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
  state_.arbitrator = false;
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

  LOGLN(face_value.target);
  LOGLN(face_value.value);

  setValueSentOnAllFaces(face_value.value);

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
    default:
      setColor(RED);
      break;
  }
}

}  // namespace state

}  // namespace blink
