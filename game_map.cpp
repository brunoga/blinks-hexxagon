#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "game_state.h"
#include "src/blinks-broadcast/bits.h"
#include "src/blinks-orientation/orientation.h"

#define GAME_MAP_MAX_BLINKS 85
#define GAME_MAP_DATAGRAM_LEN 4

namespace game {

namespace map {

struct PackedData {
  int16_t x : 6;
  int16_t y : 6;
  uint16_t player : 4;
};

static PackedData map_[GAME_MAP_MAX_BLINKS];
static byte map_index_;
static byte map_propagation_index_;

static Timer propagation_timer_;

static void propagate(const PackedData& data) {
  FOREACH_FACE(face) {
    if (isValueReceivedOnFaceExpired(face)) continue;

    byte datagram[GAME_MAP_DATAGRAM_LEN] = {
        orientation::RelativeLocalFace(face), (byte)data.x, (byte)data.y,
        (byte)data.player};

    // No need to check the return value as it is guaranteed that if we reach
    // this point, there are no pending datagrams to be sent on any face (so
    // sendDatagramOnface() will never fail).
    sendDatagramOnFace(datagram, GAME_MAP_DATAGRAM_LEN, face);
  }
}

static void maybe_propagate() {
  if (map_index_ != map_propagation_index_) {
    propagate(map_[map_propagation_index_]);
    map_propagation_index_++;
  }
}

static bool should_add_to_map(const position::Coordinates& coordinates) {
  for (byte i = 0; i < map_index_; ++i) {
    if (coordinates.x == map_[i].x && coordinates.y == map_[i].y) {
      return false;
    }
  }

  return true;
}

static void __attribute__((noinline))
add_to_map(int8_t x, int8_t y, byte player) {
  map_[map_index_].x = x;
  map_[map_index_].y = y;
  map_[map_index_].player = player;
  map_index_++;
}

static void add_local_to_map() {
  add_to_map(position::Local().x, position::Local().y,
             blink::state::GetPlayer());
}

void Process() {
  // Read any pending datagram and queue new information.
  FOREACH_FACE(face) {
    byte datagram_len = getDatagramLengthOnFace(face);
    const byte* datagram = getDatagramOnFace(face);

    if (datagram_len != GAME_MAP_DATAGRAM_LEN) {
      // Unexpected datagram. This might happen due to propagation delays for
      // the last normal broadcast message sent.
      //
      // TODO(bga): Maybe a better way to fix this is to wait for a while before
      // starting the mapping process.
      continue;
    }

    propagation_timer_.set(2000);

    if (map_index_ == 0) {
      // We do not have anything on our map, so we need to initialize our local
      // data and add it to the map.
      orientation::Setup(datagram[0], face);
      position::Setup(orientation::RelativeLocalFace(face), datagram[1],
                      datagram[2]);

      add_local_to_map();
    }

    position::Coordinates received_coordinates = {(int8_t)datagram[1],
                                                  (int8_t)datagram[2]};

    if (!should_add_to_map(received_coordinates)) {
      setColor(OFF);
      continue;
    } else {
      setColor(GREEN);
    }

    add_to_map(received_coordinates.x, received_coordinates.y, datagram[3]);
  }

  if (isDatagramPendingOnAnyFace()) {
    // Wait until all pending datagrams are sent to continue processing any
    // other data.
    return;
  }

  maybe_propagate();
}

void StartMapping(bool origin) {
  // We are the origin of the coordinates system. Reset position and
  // orientation and propagate new values.
  //
  // TODO(bga): Should add a way to propagate the reset to all Blinks in the
  // cluster. Probably should be done outside of the mapping loop so we can take
  // advantage of broadcast messages.
  Reset();

  if (origin) {
    // We are the mapping origin. Add ourselves to the map.
    add_local_to_map();
  }

  propagation_timer_.set(2000);
}

bool GetMapping() { return (!propagation_timer_.isExpired()); }

bool EmptySpaceInRange() {
  for (byte i = 0; i < map_index_; ++i) {
    if (map_[i].player == 0 &&
        position::Distance(
            position::Coordinates{(int8_t)map_[i].x, (int8_t)map_[i].y}) <= 2) {
      return true;
    }
  }

  return false;
}

byte GetBlinkCount(byte player) {
  byte blink_count = 0;
  for (byte i = 0; i < map_index_; ++i) {
    if (map_[i].player == player) blink_count++;
  }

  return blink_count;
}

byte GetPlayerCount() {
  byte players = 0;
  byte player_count = 0;
  for (byte i = 0; i < map_index_; ++i) {
    if (map_[i].player == 0) continue;

    if (!IS_BIT_SET(players, map_[i].player)) {
      SET_BIT(players, i);
      player_count++;
    }
  }

  return player_count;
}

void Reset() {
  map_index_ = 0;
  map_propagation_index_ = 0;

  orientation::Reset();
  position::Reset();
}

}  // namespace map

}  // namespace game
