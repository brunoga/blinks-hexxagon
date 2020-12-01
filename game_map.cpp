#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "game_state.h"
#include "src/blinks-broadcast/bits.h"
#include "src/blinks-orientation/orientation.h"

#define GAME_MAP_DATAGRAM_LEN 4
#define GAME_MAP_PROPAGATION_TIMEOUT 2000

namespace game {

namespace map {

struct MapData {
  int16_t x : 6;
  int16_t y : 6;
  uint16_t player : 4;
};

static MapData map_[GAME_MAP_MAX_BLINKS];
static byte map_index_;
static byte map_propagation_index_;

Stats stats_;

static Timer propagation_timer_;

static bool move_commited_;

struct MoveData {
  position::Coordinates origin;
  position::Coordinates target;
};

static MoveData move_data_;

static void propagate(MapData map_data) {
  FOREACH_FACE(face) {
    if (isValueReceivedOnFaceExpired(face)) continue;

    byte datagram[GAME_MAP_DATAGRAM_LEN] = {
        orientation::RelativeLocalFace(face), (byte)map_data.x,
        (byte)map_data.y, (byte)map_data.player};

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

static void __attribute__((noinline))
update_blinks(position::Coordinates coordinates, byte player,
              bool update_neighbors) {
  for (byte i = 0; i < map_index_; ++i) {
    if ((map_[i].x == coordinates.x) && (map_[i].y == coordinates.y)) {
      // This is the position being updated. Change it to belong to the given
      // player.
      map_[i].player = player;
    }

    if (update_neighbors && (map_[i].player != 0) &&
        (position::coordinates::Distance(
             coordinates, {(int8_t)map_[i].x, (int8_t)map_[i].y}) == 1)) {
      // Neighbor from a different player. Now belongs to the given player.
      map_[i].player = player;
    }
  }
}

void Process() {
  // Read any pending datagram and queue new information.
  FOREACH_FACE(face) {
    byte datagram_len = getDatagramLengthOnFace(face);
    const byte* datagram = getDatagramOnFace(face);
    markDatagramReadOnFace(face);

    if (datagram_len != GAME_MAP_DATAGRAM_LEN) {
      // Unexpected datagram. This might happen due to propagation delays for
      // the last normal broadcast message sent.
      //
      // TODO(bga): Maybe a better way to fix this is to wait for a while before
      // starting the mapping process.
      continue;
    }

    propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);

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
  // Mapping just started. Reset map.
  Reset();

  if (origin) {
    // We are the mapping origin. Add ourselves to the map.
    add_local_to_map();
  }

  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);
}

bool GetMapping() { return (!propagation_timer_.isExpired()); }

void ComputeMapStats() {
  memset(&stats_, 0, sizeof(Stats));

  for (byte i = 0; i < map_index_; ++i) {
    const MapData& map_data = map_[i];
    // Update number of players.
    if (map_data.player != 0 &&
        stats_.player_blink_count[map_data.player] == 0) {
      stats_.player_count++;
    }

    // Update player blink count.
    stats_.player_blink_count[map_data.player]++;

    // Update empty space in range.
    if (!(stats_.local_blink_empty_space_in_range)) {
      stats_.local_blink_empty_space_in_range =
          ((map_[i].player == 0) &&
           (position::Distance(position::Coordinates{(int8_t)map_[i].x,
                                                     (int8_t)map_[i].y}) <= 2));
    }
  }
}

static void set_data(int8_t x, int8_t y, position::Coordinates* coordinates,
                     void (*setter)(bool)) {
  coordinates->x = x;
  coordinates->y = y;

  if (x == position::Local().x && y == position::Local().y) setter(true);

  move_commited_ = false;
}

void SetMoveOrigin(int8_t x, int8_t y) {
  set_data(x, y, &move_data_.origin, blink::state::SetOrigin);
}

void SetMoveTarget(int8_t x, int8_t y) {
  set_data(x, y, &move_data_.target, blink::state::SetTarget);
}

void CommitMove() {
  if (move_commited_) return;

  if (position::coordinates::Distance(move_data_.origin, move_data_.target) >
      1) {
    update_blinks(move_data_.origin, 0, false);
  }

  update_blinks(move_data_.target, game::state::GetPlayer(), true);

  ComputeMapStats();

  move_commited_ = true;
}

const Stats& GetStats() { return stats_; }

bool __attribute__((noinline)) ValidState() {
  return (stats_.player_blink_count[0] > 0) && (stats_.player_count > 1);
}

void Reset() {
  map_index_ = 0;
  map_propagation_index_ = 0;

  ComputeMapStats();
}

}  // namespace map

}  // namespace game
