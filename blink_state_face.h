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

class ValueHandler {
 public:
  ValueHandler();
  ~ValueHandler();

  // Returns true if there are any neighbor Nlinks that belong to another
  // player.
  bool EnemyNeighbor() const;

  // Returns the highest numbered face that is requesting a map.
  byte MapRequestedFace() const;

  // Send a game reset request accross the board.
  void ResetGame();

 private:
  void InternalResetGame();

  static Value previous_value_[FACE_COUNT];

  static byte previously_connected_faces_;

  static bool reset_state_;

  byte map_requested_face_;
  bool enemy_neighbor_;
};

}  // namespace face

}  // namespace state

}  // namespace blink

#endif