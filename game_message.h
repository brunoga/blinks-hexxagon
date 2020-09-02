#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "game_state.h"
#include "message.h"

#define MESSAGE_GAME_STATE_CHANGE 0
#define MESSAGE_CHECK_BOARD 1
#define MESSAGE_GAME_STATE_PLAY_FIND_TARGETS 2
#define MESSAGE_REPORT_WINNER 3
#define MESSAGE_FLASH 4

namespace game {

namespace message {

union GameStateChangeData {
  struct {
    byte state : 2;
    byte specific_state : 4;
    byte next_player : 2;  // Add 1 for actual player number.
  };

  byte value;
};

void Setup();

void Process();

bool SendGameStateChange(byte payload);

// Indices reply: 0 = empty count, 1 = player one count, 2 = player 2 count,
// etc.
bool SendCheckBoard(broadcast::Message* reply);

// Indices reply: 0 = target found.
bool SendGameStatePlayFindTargets(broadcast::Message* reply);

// Indices message: 0 = winner player (0 if tie).
void SendReportWinner(byte winner_player);

void SendFlash();

}  // namespace message

}  // namespace game

#endif