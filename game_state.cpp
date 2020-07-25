
#include "game_state.h"

#include "debug.h"
#include "game_message.h"

namespace game {

namespace state {

struct State {
  byte current : 2;
  byte previous : 2;
  bool propagate : 1;
  bool force_propagate : 1;
  byte next_player : 2;
};
static State state_;

struct SpecificState {
  byte current : 4;
  byte previous : 4;
};
static SpecificState specific_state_;

void Set(byte state, bool propagate, bool force_propagate) {
  state_.previous = state_.current;
  state_.current = state;

  state_.propagate = propagate;
  state_.force_propagate = force_propagate;
}

byte Get() { return state_.current; }

void SetSpecific(byte specific_state, bool propagate, bool force_propagate) {
  specific_state_.previous = specific_state_.current;
  specific_state_.current = specific_state;

  state_.propagate = propagate;
  state_.force_propagate = force_propagate;
}

byte GetSpecific() { return specific_state_.current; }

void SetNextPlayer(byte next_player) { state_.next_player = next_player - 1; }

byte GetNextPlayer() { return state_.next_player + 1; }

void Reset() {
  state_.current = GAME_STATE_IDLE;
  state_.previous = GAME_STATE_IDLE;
  state_.propagate = false;
  state_.force_propagate = false;
  state_.next_player = 1;
  specific_state_.current = 0;
  specific_state_.previous = 0;
}

bool Changed(bool include_specific) {
  return include_specific
             ? state_.current != state_.previous ||
                   specific_state_.current != specific_state_.previous
             : state_.current != state_.previous;
}

bool Propagate() {
  if ((!state_.propagate || !Changed()) && !state_.force_propagate) {
    LOGFLN("already propagated");
    return true;
  }

  LOGF("propagating ");
  LOG(state_.current);
  LOGF(" ");
  LOG(specific_state_.current);
  LOGF(" ");
  LOGLN(state_.next_player);

  if (!game::message::SendGameStateChange(
          state_.current, specific_state_.current, state_.next_player + 1)) {
    return false;
  }

  LOGFLN("propagated");

  state_.propagate = false;
  state_.force_propagate = false;

  return true;
}

}  // namespace state

}  // namespace game
