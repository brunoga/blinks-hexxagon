#ifndef GAME_MESSAGE_H_
#define GAME_MESSAGE_H_

#include <blinklib.h>

#include "game_state.h"
#include "message.h"

#define MESSAGE_GAME_STATE_CHANGE 0
#define MESSAGE_CHECK_BOARD 1
#define MESSAGE_GAME_STATE_PLAY_FIND_TARGETS 2
#define MESSAGE_REPORT_WINNER 3

namespace game {

namespace message {

void Setup();

void Process();

bool SendGameStateChange(byte game_state, byte specific_state,
                         byte next_player);

// Indices reply: 0 = empty count, 1 = player one count, 2 = player 2 count,
// etc.
bool SendCheckBoard(broadcast::Message* reply);

// Indices reply: 0 = target found.
bool SendGameStatePlayFindTargets(broadcast::Message* reply);

// Indices message: 0 = winner player (0 if tie).
bool SendReportWinner(byte winner_player);

}  // namespace message

}  // namespace game

#endif