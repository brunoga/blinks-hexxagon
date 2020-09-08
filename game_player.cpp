#include "game_player.h"

namespace game {

namespace player {

static const Color colors_[] = {ORANGE, RED, BLUE, GREEN, MAGENTA};

byte __attribute__((noinline)) GetNext(byte current_player) {
  byte next_player = current_player + 1;

  return next_player > GAME_PLAYER_MAX_PLAYERS ? 0 : next_player;
}

Color __attribute__((noinline)) GetColor(byte player) {
  return colors_[player];
}

}  // namespace player

}  // namespace game