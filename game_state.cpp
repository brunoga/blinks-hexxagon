
#include "game_state.h"

#include "debug.h"
#include "game_message.h"

namespace game {

namespace state {

struct State {
  byte current : 2;
  byte previous : 2;
  byte next_player : 2;
  bool from_network : 1;
};
static State state_;

struct SpecificState {
  byte current : 4;
  byte previous : 4;
};
static SpecificState specific_state_;

void Set(byte state, bool from_network) {
  state_.previous = state_.current;
  state_.current = state;

  state_.from_network = from_network;
}

byte Get() { return state_.current; }

void SetSpecific(byte specific_state, bool from_network) {
  specific_state_.previous = specific_state_.current;
  specific_state_.current = specific_state;

  state_.from_network = from_network;
}

byte GetSpecific() { return specific_state_.current; }

void SetPlayer(byte next_player) { state_.next_player = next_player - 1; }

byte GetPlayer() { return state_.next_player + 1; }

void Reset() {
  state_.current = GAME_STATE_IDLE;
  state_.previous = GAME_STATE_IDLE;
  state_.next_player = 0;
  state_.from_network = false;
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
  if (!Changed() || state_.from_network) return true;

  if (!game::message::SendGameStateChange(
          state_.current, specific_state_.current, state_.next_player + 1)) {
    return false;
  }

  return true;
}

}  // namespace state

}  // namespace game
