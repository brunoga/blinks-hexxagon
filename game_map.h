#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include <blinklib.h>

#include "src/blinks-position/position.h"

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

bool GetMapping();

// Returns true if there is an empty space in range of the local Blink.
bool EmptySpaceInRange();

// Returns the number of Blinks in the map that belong to the given player.
byte GetBlinkCount(byte player);

byte GetPlayerCount();

// Sets all map data to invalid values.
void Reset();

}  // namespace map

}  // namespace game

#endif