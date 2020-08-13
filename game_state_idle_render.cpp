#include "game_state_idle_render.h"

#include "game_player.h"

// Disabled for now to save storage space. Reenable if we can get savings
// somewhere else.
//#define GAME_STATE_IDLE_FANCY_ANIMATION

namespace game {

namespace state {

namespace idle {

#ifdef GAME_STATE_IDLE_FANCY_ANIMATION

static Timer pulse_timer_;
static bool reverse_ = true;

void Render() {
  if (pulse_timer_.isExpired()) {
    reverse_ = !reverse_;
    pulse_timer_.set(128 * 10);
  }

  byte base_brightness = pulse_timer_.getRemaining() / 10;

  byte brightness = reverse_ ? 255 - base_brightness : base_brightness + 127;

  setColor(dim(game::player::GetColor(0), brightness));
}
#else
void Render() { setColor(dim(game::player::GetColor(0), 127)); }
#endif

}  // namespace idle

}  // namespace state

}  // namespace game