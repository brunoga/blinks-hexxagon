#include <blinklib.h>

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_end.h"
#include "game_state_idle.h"
#include "game_state_play.h"
#include "game_state_setup.h"
#include "render_animation.h"

#ifndef BGA_CUSTOM_BLINKLIB
#error \
    "This code requires a custom blinklib. See https://github.com/brunoga/blinklib/releases/latest"
#endif

void setup() { game::message::Setup(); }

void loop() {
  if (game::map::GetMapping()) {
    // We are mapping Blinks.
    game::map::Process();

    return;
  }

  // Process any pending game messages.
  game::message::Process();

  // Check escape hatch. Reset to idle state if button is long pressed.
  if (buttonLongPressed()) {
    // Ok to ignore result.
    game::message::SendFlash();

    game::state::Set(GAME_STATE_IDLE);

    return;
  }

  if (game::state::Propagate()) {
    // Cache current state and if we changed state since the previous
    // iteration.
    byte state = game::state::Get();
    byte specific_state = game::state::GetSpecific();
    bool state_changed = game::state::Changed(false);

    if (game::state::Changed()) {
      // State (including specific state) changed. Reset pulse timer to
      // improve synchronization.
      render::animation::ResetPulseTimer();
    }

    // Run our state machine.
    switch (state) {
      case GAME_STATE_IDLE:
        game::state::idle::Handler(state_changed, &state, &specific_state);
        break;
      case GAME_STATE_SETUP:
        game::state::setup::Handler(state_changed, &state, &specific_state);
        break;
      case GAME_STATE_PLAY:
        game::state::play::Handler(state_changed, &state, &specific_state);
        break;
      case GAME_STATE_END:
        game::state::end::Handler(state_changed, &state, &specific_state);
        break;
    }

    // Switch our state to the computed one. This will be propagated to other
    // Blinks in case there was a change.
    game::state::Set(state);
    game::state::SetSpecific(specific_state);
  }

  blink::state::Render(game::state::Get());

  // Consume any pending woken state. The rationale is that we already went
  // through our loop and if the state is still set here we want to consume as
  // it most likelly means another Blink woke us up. This will prevent us
  // "swallowing" the first click on this Blink after wakeup.
  hasWoken();
}
