#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <blinklib.h>

#define GAME_STATE_IDLE 0
#define GAME_STATE_SETUP 1
#define GAME_STATE_PLAY 2

namespace game {

namespace state {

void Set(byte state, bool from_network = false);
byte Get();

void SetSpecific(byte specific_state, bool from_network = false);
byte GetSpecific();

void SetPlayer(byte next_player);
byte GetPlayer();

void Reset();

bool Changed(bool include_specific = true);

bool Propagate(bool force = false);

}  // namespace state

}  // namespace game

#endif