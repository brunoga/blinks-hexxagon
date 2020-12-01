#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include <blinklib.h>

#include "game_player.h"
#include "src/blinks-position/position.h"

#define GAME_MAP_MAX_BLINKS 85

namespace game {

namespace map {

struct PlayerStats {
  byte blink_count : 7;
  bool can_move : 1;
};

struct Stats {
  byte player_count;
  PlayerStats player[GAME_PLAYER_MAX_PLAYERS + 1];
  bool local_blink_empty_space_in_range;
};

// Process should be called at the top of the mapping loop and will handle all
// incoming mapping messages.
void Process();

// Starts the mapping process. The Blink that calls it will be the coordinate
// system origin (0, 0, 0). Only a single Blink in the cluster should call this
// otherwise the behavior will be undefined and will probably break everything
// badly.
void StartMapping(bool origin);

// Returns true if the mapping process is active.
bool GetMapping();

void ComputeMapStats();

void SetMoveOrigin(int8_t x, int8_t y);

void SetMoveTarget(int8_t x, int8_t y);

// Computes the player move from origin to destination, updating the game
// map accordingly.
void CommitMove();

const Stats& GetStats();

bool ValidState();

// Sets all map data to invalid values.
void Reset();

}  // namespace map

}  // namespace game

#endif