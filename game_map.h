#ifndef GAME_MAP_H_
#define GAME_MAP_H_

#include <blinklib.h>

#include "game_player.h"
#include "src/blinks-position/position.h"

#define GAME_MAP_MAX_BLINKS 84

namespace game {

namespace map {

struct Data {
  int16_t x : 6;
  int16_t y : 6;
  uint16_t player : 4;
};

struct Statistics {
  byte player_count;
  struct {
    byte blink_count : 7;
    bool can_move : 1;
  } player[GAME_PLAYER_MAX_PLAYERS + 1];
  bool local_blink_empty_space_in_range;
};

void Setup();

// Process should be called at the top of the mapping loop and will handle all
// incoming mapping messages.
void Process();

// Starts the mapping process. The Blink that calls it will be the coordinate
// system origin (0, 0, 0). Only a single Blink in the cluster should call this
// otherwise the behavior will be undefined and will probably break everything
// badly.
void StartMapping();

// Returns true if the mapping process is active.
bool GetMapping();

void ComputeMapStats();

void SetMoveOrigin(position::Coordinates coordinates);
position::Coordinates GetMoveOrigin();

void SetMoveTarget(position::Coordinates coordinates);
position::Coordinates GetMoveTarget();

// Computes the player move from origin to destination, updating the game
// map accordingly.
void CommitMove();

const Statistics& GetStatistics();

bool ValidState();

bool MaybeUpload();
bool Uploaded();

// Sets all map data to invalid values.
void Reset();

}  // namespace map

}  // namespace game

#endif