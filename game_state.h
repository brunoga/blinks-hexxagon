#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <blinklib.h>

#include "game_player.h"

#define GAME_STATE_IDLE 0
#define GAME_STATE_SETUP 1
#define GAME_STATE_PLAY 2
#define GAME_STATE_END 3

namespace game {

namespace state {

union Data {
  struct {
    byte state : 2;
    byte specific_state : 4;
    byte next_player : 2;  // Add 1 for actual player number.
  };

  byte as_byte;
};

void Set(byte state, bool from_network = false);
byte Get();

void SetSpecific(byte specific_state, bool from_network = false);
byte GetSpecific();

void SetPlayer(byte next_player);
byte GetPlayer();
void NextPlayer();

byte GetData();

void Reset();

bool Changed(bool include_specific = true);

bool Propagate();

bool FromNetwork();

}  // namespace state

}  // namespace game

#endif