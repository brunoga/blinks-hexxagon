#include "blink_state.h"

#include <blinklib.h>

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
  byte target_type : 3;
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

void Reset() {
  state_.type = BLINK_STATE_TYPE_EMPTY;
  state_.origin = false;
  state_.target = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
}

void Render(byte game_state) {
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
