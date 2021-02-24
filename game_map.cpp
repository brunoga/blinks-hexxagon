#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_map_upload.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-broadcast/handler.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-orientation/orientation.h"

// As we can not known beforehand how many Blinks are in the map, we need a
// timeout to consider that everything cleared up. This is the time since the
// last mapping message was received.
#define GAME_MAP_PROPAGATION_TIMEOUT 2000

namespace game {

static game::map::Data map_[GAME_MAP_MAX_BLINKS];

namespace map {

static byte index_;
static byte propagation_index_;

Statistics stats_;

static Timer propagation_timer_;

static bool move_commited_;

struct MoveData {
  position::Coordinates origin;
  position::Coordinates target;
};

static MoveData move_data_;

static void maybe_propagate() {
  if (index_ <= propagation_index_) return;

  if (!game::message::SendExternalPropagateCoordinates(
          (int8_t)map_[propagation_index_].x,
          (int8_t)map_[propagation_index_].y,
          (byte)map_[propagation_index_].player)) {
    return;
  }

  propagation_index_++;
}

static bool find_entry_in_map(int8_t x, int8_t y) {
  for (byte i = 0; i < index_; ++i) {
    if (x == map_[i].x && y == map_[i].y) {
      return true;
    }
  }

  return false;
}

static void __attribute__((noinline))
add_to_map(int8_t x, int8_t y, byte player) {
  map_[index_] = {x, y, player};
  index_++;
}

static void add_local_to_map() {
  add_to_map(position::Local().x, position::Local().y,
             blink::state::GetPlayer());
}

static void update_blinks(position::Coordinates coordinates, byte player,
                          bool update_neighbors) {
  for (byte i = 0; i < index_; ++i) {
    if ((position::coordinates::Distance({(int8_t)map_[i].x, (int8_t)map_[i].y},
                                         coordinates) == 0) ||
        (update_neighbors && (map_[i].player != 0) &&
         (position::coordinates::Distance(
              coordinates, {(int8_t)map_[i].x, (int8_t)map_[i].y}) == 1))) {
      // This is either the position we are updating or we also want to update
      // neighboors and this is a non-empty neighbor (we just needed to update
      // neighbors of different players, but that would be an extra check and
      // use more storage due to that. Updating a player to itself is harmless).
      map_[i].player = player;
    }
  }
}

void consume(const broadcast::Message* message, byte local_absolute_face) {
  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);

  if (index_ == 0) {
    // We do not have anything on our map, so we need to initialize our
    // local data and add it to the map.
    orientation::Setup(message->payload[0], local_absolute_face);
    position::Setup(orientation::RelativeLocalFace(local_absolute_face),
                    (int8_t)message->payload[1], (int8_t)message->payload[2]);

    add_local_to_map();
  }

  if (find_entry_in_map((int8_t)message->payload[1],
                        (int8_t)message->payload[2])) {
    return;
  }

  add_to_map((int8_t)message->payload[1], (int8_t)message->payload[2],
             message->payload[3]);
}

void Setup() {
  broadcast::message::handler::Set(
      {MESSAGE_EXTERNAL_PROPAGATE_COORDINATES, consume});
}

void Process() { maybe_propagate(); }

void StartMapping() {
  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);

  // We are the mapping origin. Add ourselves to the map.
  add_local_to_map();
}

bool GetMapping() { return (!propagation_timer_.isExpired()); }

void ComputeMapStats() {
  memset(&stats_, 0, sizeof(Statistics));

  for (byte i = 0; i < index_; ++i) {
    const Data& map_data = map_[i];

    // Update player blink count.
    stats_.player[map_data.player].blink_count++;

    // Update number of players.
    if (map_data.player != 0 &&
        stats_.player[map_data.player].blink_count == 1) {
      stats_.player_count++;
    }

    // Update player can move.
    for (byte j = 0; j < index_; ++j) {
      if ((map_[j].player == GAME_PLAYER_NO_PLAYER) &&
          (position::coordinates::Distance(
               {(int8_t)map_data.x, (int8_t)map_data.y},
               {(int8_t)map_[j].x, (int8_t)map_[j].y}) <= 2)) {
        stats_.player[map_data.player].can_move = true;
        break;
      }
    }

    // Update empty space in range.
    if (!(stats_.local_blink_empty_space_in_range)) {
      stats_.local_blink_empty_space_in_range =
          ((map_data.player == GAME_PLAYER_NO_PLAYER) &&
           (position::Distance(position::Coordinates{
                (int8_t)map_data.x, (int8_t)map_data.y}) <= 2));
    }
  }
}

const Data* Get() { return map_; }

byte GetSize() { return index_; }

void SetMoveOrigin(position::Coordinates coordinates) {
  move_data_.origin = coordinates;

  move_commited_ = false;
}

position::Coordinates GetMoveOrigin() { return move_data_.origin; }

void SetMoveTarget(position::Coordinates coordinates) {
  move_data_.target = coordinates;

  move_commited_ = false;
}

position::Coordinates GetMoveTarget() { return move_data_.target; }

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

const Statistics& GetStatistics() { return stats_; }

bool ValidState() {
  return (stats_.player[0].blink_count > 0) && (stats_.player_count > 1);
}

void Reset() {
  index_ = 0;
  propagation_index_ = 0;

  ComputeMapStats();

  game::map::upload::Reset();
}

}  // namespace map

}  // namespace game
