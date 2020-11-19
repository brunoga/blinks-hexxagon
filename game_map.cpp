#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "game_state.h"
#include "src/blinks-orientation/orientation.h"
#include "src/blinks-position/position.h"

#define GAME_MAP_MAX_BLINKS 128
#define GAME_MAP_INVALID_COORDINATE -128

namespace game {

namespace map {

struct Data {
  position::Coordinates coordinates;
  byte player;
};

static Data map_[GAME_MAP_MAX_BLINKS];

static int8_t last_received_index_ = -1;
static int8_t last_propagated_index_ = -1;

static bool initialized_;

static void propagate(const position::Coordinates& coordinates, byte player) {
  FOREACH_FACE(face) {
    if (isValueReceivedOnFaceExpired(face)) continue;

    byte datagram[4] = {orientation::RelativeLocalFace(face),
                        (byte)coordinates.x, (byte)coordinates.y, player};

    sendDatagramOnFace(datagram, 4, face);
  }
}

static bool is_new_data(int8_t x, int8_t y) {
  if (x == position::Local().x && y == position::Local().y) {
    return false;
  }

  for (byte i = 0; i < GAME_MAP_MAX_BLINKS; ++i) {
    if (map_[i].coordinates.x == GAME_MAP_INVALID_COORDINATE) break;

    if (x == map_[i].coordinates.x && y == map_[i].coordinates.y) {
      return false;
    }
  }

  return true;
}

void Process() {
  // Read any pending datagram and queue new information.
  FOREACH_FACE(face) {
    if (getDatagramLengthOnFace(face) != 4) continue;

    const byte* datagram = getDatagramOnFace(face);

    if (!initialized_) {
      orientation::Setup(datagram[0], face);
      position::Setup(datagram[0], datagram[1], datagram[2]);

      propagate(position::Local(), blink::state::GetPlayer());

      initialized_ = true;
    }

    if (!is_new_data(datagram[1], datagram[2])) continue;

    last_received_index_++;
    map_[last_received_index_].coordinates =
        position::Coordinates{datagram[1], datagram[2]};
    map_[last_received_index_].player = datagram[3];
  }

  if (isDatagramPendingOnAnyFace()) {
    // Wait until all pending datagrams are sent to continue processing any
    // other data.
    return;
  }

  if (last_propagated_index_ < last_received_index_) {
    // Forward a single new pending coordinate. This together with the check
    // above guarantees that sending will never fail.
    last_propagated_index_++;

    propagate(map_[last_propagated_index_].coordinates,
              map_[last_propagated_index_].player);
  }
}

void StartMapping() {
  // We are the origin of the coordinates system. Reset position and
  // orientation and propagate new values.
  orientation::Reset();
  position::Reset();

  propagate(position::Local(), blink::state::GetPlayer());

  initialized_ = true;
}

void StopMapping() { game::state::SetMapping(false); }

bool EmptySpaceInRange() {
  for (byte i = 0; i <= last_received_index_; ++i) {
    if (map_[i].player == 0 && position::Distance(map_[i].coordinates) <= 2) {
      return true;
    }
  }

  return false;
}

void Reset() {
  memset(map_, GAME_MAP_INVALID_COORDINATE, GAME_MAP_MAX_BLINKS * sizeof(Data));
}

}  // namespace map

}  // namespace game
