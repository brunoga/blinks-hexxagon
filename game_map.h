#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include <blinklib.h>

#include "src/blinks-position/position.h"

#define GAME_MAP_MAX_BLINKS 80

namespace game {

namespace map {

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

// Returns true if there is an empty space in range of the local Blink.
bool EmptySpaceInRange();

byte GetBlinkCount();

// Returns the number of Blinks in the map that belong to the given player.
byte GetBlinkCount(byte player);

// Returns the number of different players in the map.
byte GetPlayerCount();

bool ValidState();

// Sets all map data to invalid values.
void Reset();

}  // namespace map

}  // namespace game

#endif