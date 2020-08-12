#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "message.h"

#define MESSAGE_GAME_STATE_CHANGE 0
#define MESSAGE_CHECK_BOARD 1
#define MESSAGE_GAME_STATE_PLAY_FIND_TARGETS 2

namespace game {

namespace message {

void Setup();

void Process();

bool SendGameStateChange(byte game_state, byte specific_state,
                         byte next_player);

// Indexes: 0 = node count, 1 = player one count, 2 = player 2 count.
bool SendCheckBoard(broadcast::Message* reply);

// Indexes: 0 = target found.
bool SendGameStatePlayFindTargets(broadcast::Message* reply);

}  // namespace message

}  // namespace game

#endif