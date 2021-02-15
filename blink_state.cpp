#include "blink_state.h"

#include <blinklib.h>

#include "game_state.h"
#include "game_state_end_render.h"
#include "game_state_idle_render.h"
#include "game_state_play_render.h"
#include "game_state_setup_render.h"

#if __has_include(<hexxagon_config.h>)
#include <hexxagon_config.h>
#endif

namespace blink {

struct State {
  bool origin;
  bool target;
  byte player;
  byte target_type;
};
static State state_;

namespace state {

static Timer color_override_timer_;

void SetOrigin(bool origin) { state_.origin = origin; }
bool __attribute__((noinline)) GetOrigin() { return state_.origin; }

void SetTarget(bool target) { state_.target = target; }
bool GetTarget() { return state_.target; }

void SetTargetType(byte target_type) { state_.target_type = target_type; }
byte GetTargetType() { return state_.target_type; }

void SetPlayer(byte player) { state_.player = player; }
byte GetPlayer() { return state_.player; }

void StartColorOverride() { color_override_timer_.set(200); }

bool __attribute__((noinline)) GetColorOverride() {
  return !color_override_timer_.isExpired();
}

void __attribute__((noinline)) Reset() {
  state_.origin = false;
  state_.target = false;
  state_.target_type = BLINK_STATE_TARGET_TYPE_NONE;
  state_.player = 0;
}

void Render(byte game_state) {
  if (GetColorOverride()) {
    setColor(WHITE);
  } else {
    // Now render the state specific colors/animations.
    if (game_state < GAME_STATE_SETUP) {
      game::state::idle::Render();
    } else if (game_state < GAME_STATE_PLAY) {
      game::state::setup::Render();
    } else if (game_state < GAME_STATE_END) {
      game::state::play::Render();
    } else {
      game::state::end::Render();
    }
  }

#ifdef HEXXAGON_ENABLE_PENDING_FACE_DEBUG
  FOREACH_FACE(face) {
    if (isDatagramPendingOnFace(face)) {
      setColorOnFace(OFF, face);
    }
  }
#endif  // HEXXAGON_ENABLE_PENDING_FACE_DEBUG
}

}  // namespace state

}  // namespace blink
