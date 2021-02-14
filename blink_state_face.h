#ifndef BLINK_STATE_FACE_H_
#define BLINK_STATE_FACE_H_

#include <blinklib.h>

namespace blink {

namespace state {

namespace face {

union Value {
  struct {
    bool unused : 2;
    bool color_override : 1;  // If true, start color override.
    bool reset_state : 1;     // If this bit changes, then we should reset.
    bool map_requested : 1;   // Map data was requested on this face.
    byte player : 3;          // 0 means empty.
  };

  byte as_byte;
};

namespace handler {

// Must be called at the top of loop().
void ProcessTop();

// Must be called at the bottom of the loop and must always be executed (careful
// with early returns).
void ProcessBottom();

// Returns true if there are any neighbor Nlinks that belong to another
// player.
bool EnemyNeighbor();

// Returns the highest numbered face that is requesting a map.
byte MapRequestedFace();

// Returns true if this face is connected/disconnected as expected. False if
// it is connected and was supposed to be disconnected or vice-versa.
bool FaceOk(byte face);

// Send a game reset request accross the board.
void ResetGame();

}  // namespace handler

}  // namespace face

}  // namespace state

}  // namespace blink

#endif