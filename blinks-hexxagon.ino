#include <blinklib.h>
#include <hexxagon_config.h>

#include "blink_state.h"
#include "blink_state_face.h"
#include "game_map.h"
#include "game_map_upload.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_end.h"
#include "game_state_idle.h"
#include "game_state_play.h"
#include "game_state_setup.h"
#include "src/blinks-broadcast/manager.h"

#ifndef BGA_CUSTOM_BLINKLIB
#error \
    "This code requires a custom blinklib. See https://github.com/brunoga/blinklib/releases/latest"
#endif

void setup() {
  game::message::Setup();
  game::map::Setup();
}

void loop() {
  blink::state::face::handler::ProcessTop();

  if (!game::map::upload::Process()) {
    // Process any pending game messages.
    broadcast::manager::Process();

    // Cache current state.
    byte state = game::state::Get();

    // Check escape hatch. Reset to idle state if button is long pressed.
#ifdef HEXXAGON_MULTI_CLICK_RESET
    if (buttonMultiClicked()) {
#else
    if (buttonLongPressed()) {
#endif
      blink::state::face::handler::ResetGame();

      return;
    }

    if (game::state::Propagate()) {
      // Run our state machine.
      if (state < GAME_STATE_SETUP) {
        game::state::idle::Handler(&state);
      } else if (state < GAME_STATE_PLAY) {
        game::state::setup::Handler(&state);
      } else if (state < GAME_STATE_END) {
        game::state::play::Handler(&state);
      } else {
        game::state::end::Handler(&state);
      }

      // Switch our state to the computed one.This will be propagated to other
      // Blinks in case there was a change.
      game::state::Set(state);
    }
  }

  blink::state::Render(game::state::Get());

  // Consume any pending woken state. The rationale is that we already went
  // through our loop and if the state is still set here we want to consume as
  // it most likelly means another Blink woke us up. This will prevent us
  // "swallowing" the first click on this Blink after wakeup.
  hasWoken();

  blink::state::face::handler::ProcessBottom();
}
