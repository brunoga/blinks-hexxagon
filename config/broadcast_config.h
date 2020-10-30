#ifndef BROADCAST_CONFIG_H_
#define BROADCAST_CONFIG_H_

#include "../game_player.h"

// This is the maximum payload we will need.
#if (GAME_PLAYER_MAX_PLAYERS + 1) > 4
// MESSAGE_CHECK_BOARD_STATE/MESSAGE_REPORT_BOARD_STATE
#define MESSAGE_PAYLOAD_BYTES GAME_PLAYER_MAX_PLAYERS + 1
#else
// MESSAGE_
#define MESSAGE_PAYLOAD_BYTES 4
#endif

#endif
