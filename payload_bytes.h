#ifndef PAYLOAD_BYTES_H_
#define PAYLOAD_BYTES_H_

#include "game_player.h"

// This is the maximum payload we will need which is 5 bytes for the
// MESSAGE_CHECK_BOARD message (considering a maximum of 4 players). If we have
// 3 players or less, then the biggest message is the
// MESSAGE_GAME_STATE_PLAY_FIND_TARGETS with a payload of 3.
#if GAME_PLAYER_MAX_PLAYERS > 3
#define MESSAGE_PAYLOAD_BYTES GAME_PLAYER_MAX_PLAYERS + 1
#else
#define MESSAGE_PAYLOAD_BYTES 3
#endif

#endif  // PAYLOAD_BYTES_H_