#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <blinklib.h>

#define GAME_STATE_IDLE 0
#define GAME_STATE_SETUP 1
#define GAME_STATE_PLAY 2

namespace game {

namespace state {

void Set(byte state, bool propagate = false, bool force_propagate = false);
byte Get();

void SetSpecific(byte specific_state, bool propagate = false,
                 bool force_propagate = false);
byte GetSpecific();

void SetNextPlayer(byte next_player);
byte GetNextPlayer();

void Reset();

bool Changed(bool include_specific = true);

bool Propagate();

}  // namespace state

}  // namespace game

#endif