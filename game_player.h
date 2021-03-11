#ifndef GAME_PLAYER_H_
#define GAME_PLAYER_H_

#include <blinklib.h>

// Including 0 (no player)
#define GAME_PLAYER_MAX_PLAYERS 5

#define GAME_PLAYER_NO_PLAYER 0

namespace game {

namespace player {

byte GetNext(byte current_player);
Color GetColor(byte player);
bool GetLitFace(byte player, byte face);

}  // namespace player

}  // namespace game

#endif  // GAME_PLAYER_H_