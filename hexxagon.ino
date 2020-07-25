#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_idle.h"
#include "game_state_play.h"
#include "game_state_setup.h"

extern int __bss_end;
extern void *__brkval;

int get_free_memory() {
  int free_memory;

  if ((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}

void setup() { game::message::Setup(); }

void logState() {
  LOG(game::state::Get());
  LOGF(" ");
  LOGLN(game::state::GetSpecific());
}

void loop() {
  LOGF("loop top ");
  logState();

  // Process any pending game messages.
  game::message::Process();

  LOGF("after process ");
  logState();

  if (game::state::Propagate()) {
    // Cache current state and if we changed state since the previous iteration.
    byte state = game::state::Get();
    bool state_changed = game::state::Changed(false);

    // Check escape hatch. Reset to idle state if button is long pressed.
    if (buttonLongPressed()) {
      game::state::Reset();
      blink::state::Reset();

      game::state::Set(GAME_STATE_IDLE, true, true);

      return;
    }

    // Run our state machine.
    switch (state) {
      case GAME_STATE_IDLE:
        state = game::state::idle::Handler(state_changed);
        break;
      case GAME_STATE_SETUP:
        state = game::state::setup::Handler(state_changed);
        break;
      case GAME_STATE_PLAY:
        state = game::state::play::Handler(state_changed);
        break;
    }

    LOGF("after switch ");
    logState();

    // Switch our state to the computed one and enable propagation.
    game::state::Set(state, true);

    LOGLN(game::state::Changed());
  }

  blink::state::Render(game::state::Get());

  // LOGLN(get_free_memory());
}
