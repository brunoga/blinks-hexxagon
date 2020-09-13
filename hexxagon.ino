#include <blinklib.h>

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_end.h"
#include "game_state_idle.h"
#include "game_state_play.h"
#include "game_state_setup.h"
#include "manager.h"
#include "render_animation.h"

#ifndef BGA_CUSTOM_BLINKLIB
#error "This code requires BGA's Custom Blinklib"
#endif

void setup() { game::message::Setup(); }

void loop() {
  if (isAlone()) {
    // We are alone. Make it evident.
    //
    // TODO(bga): Removing this saves 58 bytes of storage. We do not need it now
    // but if storage pressure mounts again, we can just eliminate this entire
    // block. It is a nice to have but has no gameplay effect whatsoever.
    render::animation::Pulse(WHITE, 63, 2);

    return;
  }

  // Process any pending game messages.
  game::message::Process();

  // Check escape hatch. Reset to idle state if button is long pressed.
  if (buttonLongPressed()) {
    // Ok to ignore result.
    game::message::SendFlash();

    game::state::Set(GAME_STATE_IDLE);
  }

  if (game::state::Propagate()) {
    // Cache current state and if we changed state since the previous
    // iteration.
    byte state = game::state::Get();
    byte specific_state = game::state::GetSpecific();
    bool state_changed = game::state::Changed(false);

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
