#ifndef BLINK_STATE_FACE_H_
#define BLINK_STATE_FACE_H_

#include <blinklib.h>

namespace blink {

namespace state {

namespace face {

union Value {
  struct {
    bool unused : 3;
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

  // Return the player associated with the Blink connected at the given face.
  // Note it will return 0 if there is no Blink connected (and also if there is
  // a Blink connected but it has no associated player).
  byte GetPlayerAtFace(byte face) const;

  // True if the given face was disconnected the previous loop iteration and is
  // connected now.
  bool FaceConnected(byte face) const;

  // True if the given face was connected the previous loop iteration and is
  // disconnected now.
  bool FaceDisconnected(byte face) const;

  // Returns the highest numbered face that is requesting a map.
  bool MapRequestedFace() const;

  // Send a game reset request accross the board.
  void ResetGame();

 private:
  void InternalResetGame(bool new_reset_state);

  static Value previous_value_[FACE_COUNT];

  static byte previously_connected_faces_;
  static byte currently_connected_faces_;

  byte map_requested_face_;
  bool reset_state_;
};

}  // namespace face

}  // namespace state

}  // namespace blink

#endif