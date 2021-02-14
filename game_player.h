#ifndef GAME_PLAYER_H_
#define GAME_PLAYER_H_

#include <blinklib.h>

// Including 0 (no player)
#define GAME_PLAYER_MAX_PLAYERS 5

// Players.
#define GAME_PLAYER_NO_PLAYER 0
#define GAME_PLAYER_RED_PLAYER 1
#define GAME_PLAYER_BLUE_PLAYER 2
#define GAME_PLAYER_GREEN_PLAYER 3
#define GAME_PLAYER_MAGENTA_PLAYER 4

namespace game {

namespace player {

byte GetNext(byte current_player);
Color GetColor(byte player);

}  // namespace player

}  // namespace game

#endif  // GAME_PLAYER_H_