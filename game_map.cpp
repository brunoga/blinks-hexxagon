#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_map_upload.h"
#include "game_message.h"
#include "game_state.h"
#include "game_state_play.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-mapping/mapping.h"
#include "src/blinks-orientation/orientation.h"

// As we can not known beforehand how many Blinks are in the map, we need a
// timeout to consider that everything cleared up. This is the time since the
// last mapping message was received.
#define GAME_MAP_PROPAGATION_TIMEOUT 2000

// We use the most significant bit to indicate if the value is new (has not been
// propagated yet) or not. This is only relevant during map generation but this
// means we can only use values from 1 (0 is reserved) to 127 (which is fine
// as we only use 5 values anyway).
#define GAME_MAP_SET_NEW_VALUE(v) ((v) | 0b10000000)
#define GAME_MAP_IS_NEW_VALUE(v) ((v)&0b10000000)
#define GAME_MAP_RESET_NEW_VALUE(v) ((v)&0b01111111)

namespace game {

namespace map {

Statistics stats_;

static Timer propagation_timer_;

static bool move_commited_;

struct MoveData {
  position::Coordinates origin;
  position::Coordinates target;
};

static MoveData move_data_;

static bool propagate_value(int8_t x, int8_t y, byte maybe_new_value) {
  if (GAME_MAP_IS_NEW_VALUE(maybe_new_value)) {
    byte value = GAME_MAP_RESET_NEW_VALUE(maybe_new_value);
    if (game::message::SendExternalPropagateCoordinates(x, y, value)) {
      mapping::Set(x, y, value);
    }

    return true;
  }

  return false;
}

static void maybe_propagate() {
  if (!mapping::Initialized()) return;

  byte value = mapping::Get(position::Local().x, position::Local().y);
  if (propagate_value(position::Local().x, position::Local().y, value)) {
    return;
  }

  mapping::Iterator iterator;
  int8_t x;
  int8_t y;
  while (byte value = mapping::GetNextValidPosition(&iterator, &x, &y)) {
    if (propagate_value(x, y, value)) break;
  }
}

static void update_blinks(position::Coordinates coordinates, byte player,
                          bool update_neighbors) {
  mapping::Set(coordinates.x, coordinates.y, player + 1);

  if (update_neighbors) {
    mapping::Iterator iterator;
    int8_t x;
    int8_t y;
    while (byte value = mapping::GetNextValidPositionAround(
               coordinates.x, coordinates.y, 1, &iterator, &x, &y)) {
      byte map_player = value - 1;
      if (map_player != GAME_PLAYER_NO_PLAYER) mapping::Set(x, y, player + 1);
    }
  }
}

bool external_message_handler(byte local_absolute_face,
                              const broadcast::Message* message) {
  if (message->header.id != MESSAGE_EXTERNAL_PROPAGATE_COORDINATES) {
    return false;
  }

  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);

  if (!mapping::Initialized()) {
    // We do not have anything on our map, so we need to initialize our local
    // data and add it to the map.
    orientation::Setup(message->payload[0], local_absolute_face);
    position::Setup(orientation::RelativeLocalFace(local_absolute_face),
                    (int8_t)message->payload[1], (int8_t)message->payload[2]);

    mapping::Set(position::Local().x, position::Local().y,
                 GAME_MAP_SET_NEW_VALUE(blink::state::GetPlayer() + 1));
  }

  if (!mapping::Get((int8_t)message->payload[1], (int8_t)message->payload[2])) {
    mapping::Set((int8_t)message->payload[1], (int8_t)message->payload[2],
                 GAME_MAP_SET_NEW_VALUE((int8_t)message->payload[3]));
  }

  return true;
}

void Process() { maybe_propagate(); }

void StartMapping() {
  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);

  // We are the mapping origin. Add ourselves to the map.
  mapping::Set(position::Local().x, position::Local().y,
               GAME_MAP_SET_NEW_VALUE(blink::state::GetPlayer() + 1));
}

bool GetMapping() { return (!propagation_timer_.isExpired()); }

void ComputeMapStats() {
  memset(&stats_, 0, sizeof(Statistics));

  byte max_player_blinks = 0;

  mapping::Iterator iterator;
  int8_t x;
  int8_t y;
  while (byte value = mapping::GetNextValidPosition(&iterator, &x, &y)) {
    byte map_player = value - 1;

    // Update player blink count.
    stats_.player[map_player].blink_count++;

    if (map_player != GAME_PLAYER_NO_PLAYER) {
      // Update number of players.
      if (stats_.player[map_player].blink_count == 1) {
        stats_.player_count++;
      }

      // Keep track of currently winning players.
      byte player_mask = 1 << map_player;
      byte player_blink_count = stats_.player[map_player].blink_count;

      if (player_blink_count >= max_player_blinks) {
        stats_.winning_players_mask =
            player_blink_count == max_player_blinks
                ? stats_.winning_players_mask | player_mask
                : player_mask;
        max_player_blinks = player_blink_count;
      }
    }

    // Update player can move.
    mapping::Iterator iterator2;
    int8_t x2;
    int8_t y2;
    while (byte value = mapping::GetNextValidPositionAround(x, y, 2, &iterator2,
                                                            &x2, &y2)) {
      byte map_player_2 = value - 1;
      if (map_player_2 == GAME_PLAYER_NO_PLAYER) {
        stats_.player[map_player].can_move = true;
        break;
      }
    }

    // Update empty space in range.
    //
    // TODO(bga): Double chjeck this.
    if (!(stats_.local_blink_empty_space_in_range)) {
      stats_.local_blink_empty_space_in_range =
          ((map_player == GAME_PLAYER_NO_PLAYER) &&
           (position::Distance(position::Coordinates{x, y}) <= 2));
    }
  }
}

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
  game::map::upload::Reset();
  mapping::Reset();

  ComputeMapStats();
}

}  // namespace map

}  // namespace game
