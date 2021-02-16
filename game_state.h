#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <blinklib.h>

#include "game_player.h"

// Idle specific state.
#define GAME_STATE_IDLE 0

// Setup specific states.
#define GAME_STATE_SETUP 1
#define GAME_STATE_SETUP_SELECT_PLAYERS 2
#define GAME_STATE_SETUP_MAP 3
#define GAME_STATE_SETUP_VALIDATE 4

// Play specific states.
#define GAME_STATE_PLAY 5
#define GAME_STATE_PLAY_SELECT_ORIGIN 6
#define GAME_STATE_PLAY_ORIGIN_SELECTED 7
#define GAME_STATE_PLAY_SELECT_TARGET 8
#define GAME_STATE_PLAY_TARGET_SELECTED 9
#define GAME_STATE_PLAY_MOVE_CONFIRMED 10
#define GAME_STATE_PLAY_RESOLVE_MOVE 11

// End specific state.
#define GAME_STATE_END 12

namespace game {

namespace state {

union Data {
  struct {
    byte unused : 1;
    byte state : 4;
    byte next_player : 3;
  };

  byte as_byte;
};

void Set(byte state, bool from_network = false);
byte Get();

void SetPlayer(byte next_player);
byte GetPlayer();
void NextPlayer();

void SetWinnerPlayer(byte winner_player);
byte GetWinnerPlayer();

byte GetData();

void Reset();

bool Changed();

bool Propagate();

bool FromNetwork();

}  // namespace state

}  // namespace game

#endif