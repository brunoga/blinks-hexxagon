#include <blinklib.h>

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
#include "render_animation.h"
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
  blink::state::face::ValueHandler face_value_handler;

  if (!game::map::upload::Process(face_value_handler)) {
    // Process any pending game messages.
    broadcast::manager::Process();

    // Check escape hatch. Reset to idle state if button is long pressed.
    if (buttonLongPressed()) {
      blink::state::StartColorOverride();
      face_value_handler.ResetGame();

      return;
    }

    if (game::state::Propagate()) {
      // Cache current state.
      byte state = game::state::Get();

      if (game::state::Changed()) {
        // State changed. Reset animation timer to improve synchronization.
        render::animation::ResetTimer();
      }

      // Run our state machine.
      if (state < GAME_STATE_SETUP) {
        game::state::idle::Handler(&state);
      } else if (state < GAME_STATE_PLAY) {
        game::state::setup::Handler(&state);
      } else if (state < GAME_STATE_END) {
        game::state::play::Handler(&state, face_value_handler);
      } else {
        game::state::end::Handler(&state, &face_value_handler);
      }

      // Switch our state to the computed one. This will be propagated to other
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
}
