#ifndef GAME_PLAYER_H_
#define GAME_PLAYER_H_

#include <blinklib.h>

#define GAME_PLAYER_MAX_PLAYERS 2

namespace game {

namespace player {

byte GetNext(byte current_player);
Color GetColor(byte player);

}  // namespace player

}  // namespace game

#endif  // GAME_PLAYER_H_