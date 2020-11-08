#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "game_state.h"
#include "src/blinks-broadcast/message.h"

#define MESSAGE_GAME_STATE_CHANGE 1
#define MESSAGE_CHECK_BOARD_STATE 2
#define MESSAGE_REPORT_BOARD_STATE 3
#define MESSAGE_FIND_TARGETS 4
#define MESSAGE_FLASH 5

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
bool SendCheckBoardState(broadcast::Message* reply);

bool SendReportBoardState();

// Indices reply: 0 = target found.
bool SendFindTargets(broadcast::Message* reply);

bool SendFlash();

bool SendReset();

}  // namespace message

}  // namespace game

#endif