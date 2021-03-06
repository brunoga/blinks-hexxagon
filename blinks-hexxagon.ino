#include <blinklib.h>

#include "blink_state.h"
#include "blink_state_face.h"
#include "config/hexxagon_config.h"
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

void setup() {}

void loop() {
  blink::state::face::handler::ProcessTop();

  // Consume all relevant state flags.
  bool has_woken = hasWoken();
  bool button_single_clicked = buttonSingleClicked() & !has_woken;
  bool button_double_clicked = buttonDoubleClicked();

  if (!game::map::upload::Process()) {
    // Process any pending game messages.
    broadcast::manager::Process();

    // Cache current state.
    byte state = game::state::Get();

    // Check escape hatch. Reset to idle state if button is long pressed.
    if (buttonMultiClicked()) {
      blink::state::face::handler::ResetGame();

      return;
    }

    if (game::state::Propagate()) {
      // Run our state machine.
      if (state < GAME_STATE_SETUP) {
        game::state::idle::Handler(&state, button_single_clicked);
      } else if (state < GAME_STATE_PLAY) {
        game::state::setup::Handler(&state, button_single_clicked,
                                    button_double_clicked);
      } else if (state < GAME_STATE_END) {
        game::state::play::Handler(&state, button_single_clicked);
      } else {
        game::state::end::Handler(&state, button_double_clicked);
      }

      // Switch our state to the computed one.This will be propagated to other
      // Blinks in case there was a change.
      game::state::Set(state);
    }
  }

  blink::state::Render(game::state::Get());

  blink::state::face::handler::ProcessBottom();
}
