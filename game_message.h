#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "game_state.h"
#include "src/blinks-broadcast/message.h"

#define MESSAGE_GAME_STATE_CHANGE 1
#define MESSAGE_SELECT_ORIGIN 2
#define MESSAGE_SELECT_TARGET 3
#define MESSAGE_EXTERNAL_PROPAGATE_COORDINATES 4  // External handler.
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

bool SendGameStateChange(byte payload);

bool SendSelectOrigin(int8_t x, int8_t y);

bool SendSelectTarget(int8_t x, int8_t y);

bool SendExternalPropagateCoordinates(int8_t x, int8_t y, byte player);

bool SendFlash();

}  // namespace message

}  // namespace game

#endif