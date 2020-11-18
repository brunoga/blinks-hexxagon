#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <blinklib.h>

#include "game_player.h"

#define GAME_STATE_IDLE 0
#define GAME_STATE_SETUP 1
#define GAME_STATE_PLAY 2
#define GAME_STATE_END 3

#define GAME_STATE_UPDATE_BOARD_STATE_OK 0
#define GAME_STATE_UPDATE_BOARD_STATE_UPDATING 1
#define GAME_STATE_UPDATE_BOARD_STATE_ERROR 2

namespace game {

namespace state {

void Set(byte state, bool from_network = false);
byte Get();

void SetSpecific(byte specific_state, bool from_network = false);
byte GetSpecific();

void SetPlayer(byte next_player);
byte GetPlayer();
void NextPlayer();

byte UpdateBoardState();

void SetBlinkCount(byte* blink_count);
byte* GetBlinkCount();

void Reset();

bool Changed(bool include_specific = true);

bool Propagate();

bool FromNetwork();

void SetMapping(bool mapping);
bool Mapping();

}  // namespace state

}  // namespace game

#endif