#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_end.h"
#include "game_state_idle.h"
#include "game_state_play.h"
#include "game_state_setup.h"

void setup() {
  game::message::Setup();
  blink::state::Reset();
  game::state::Reset();
}

void loop() {
  // Process any pending game messages.
  game::message::Process();

  // Check escape hatch. Reset to idle state if button is long pressed.
  if (buttonLongPressed()) {
    game::state::Set(GAME_STATE_IDLE);
  }

  if (game::state::Propagate(/*force_propagation_*/)) {
    // Cache current state and if we changed state since the previous iteration.
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
    // nodes in case there was a change.
    game::state::Set(state);
    game::state::SetSpecific(specific_state);
  }

  blink::state::Render(game::state::Get());

  buttonSingleClicked();
  buttonDoubleClicked();
  buttonLongPressed();
}
