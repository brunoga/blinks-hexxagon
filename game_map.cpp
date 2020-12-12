#include "game_map.h"

#include <string.h>

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
#include "src/blinks-broadcast/handler.h"
#include "src/blinks-broadcast/manager.h"
#include "src/blinks-orientation/orientation.h"

// As we can not known beforehand how many Blinks are in the map, we need a
// timeout to consider that everything cleared up. This is the time since the
// last mapping message was received.
#define GAME_MAP_PROPAGATION_TIMEOUT 2000

#define GAME_MAP_UPLOAD_STATE_SEND_METADATA 0
#define GAME_MAP_UPLOAD_STATE_UPLOAD 1

#define GAME_MAP_UPLOAD_MAX_CHUNK_SIZE 5

namespace game {

static game::map::Data map_[GAME_MAP_MAX_BLINKS];

namespace map {

static byte index_;
static byte propagation_index_;
static byte upload_index_;
static byte upload_state_;

Statistics stats_;

static Timer propagation_timer_;

static bool move_commited_;

struct MoveData {
  position::Coordinates origin;
  position::Coordinates target;
};

static MoveData move_data_;

static void maybe_propagate() {
  if (index_ != propagation_index_) {
    if (game::message::SendExternalPropagateCoordinates(
            (int8_t)map_[propagation_index_].x,
            (int8_t)map_[propagation_index_].y,
            (byte)map_[propagation_index_].player)) {
      propagation_index_++;
    }
  }
}

static Data* find_entry_in_map(int8_t x, int8_t y) {
  for (byte i = 0; i < index_; ++i) {
    if (x == map_[i].x && y == map_[i].y) {
      return &map_[i];
    }
  }

  return nullptr;
}

static void __attribute__((noinline))
add_to_map(int8_t x, int8_t y, byte player) {
  map_[index_].x = x;
  map_[index_].y = y;
  map_[index_].player = player;
  index_++;
}

static void add_local_to_map() {
  add_to_map(position::Local().x, position::Local().y,
             blink::state::GetPlayer());
}

static void __attribute__((noinline))
update_blinks(position::Coordinates coordinates, byte player,
              bool update_neighbors) {
  for (byte i = 0; i < index_; ++i) {
    if (((map_[i].x == coordinates.x) && (map_[i].y == coordinates.y)) ||
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
                        (int8_t)message->payload[2]) != nullptr) {
    setColor(OFF);

    return;
  }

  setColor(game::player::GetColor(blink::state::GetPlayer()));

  add_to_map((int8_t)message->payload[1], (int8_t)message->payload[2],
             message->payload[3]);
}

static void update_map_requested_face() {
  byte map_requested_face = blink::state::GetMapRequestedFace();

  // Check if we are currently connected.
  if (map_requested_face != FACE_COUNT) {
    // We are, did we get disconnected?
    if (isValueReceivedOnFaceExpired(map_requested_face)) {
      upload_index_ = 0;
      upload_state_ = GAME_MAP_UPLOAD_STATE_SEND_METADATA;

      // Make sure we will not leave data hanging if disconnected in the middle
      // of a transfer.
      resetPendingDatagramOnFace(map_requested_face);

      blink::state::SetMapRequestedFace(FACE_COUNT);
    }

    return;
  }

  FOREACH_FACE(face) {
    // Parse face value so we can check for a map being requested.
    blink::state::FaceValue face_value = {.as_byte =
                                              getLastValueReceivedOnFace(face)};

    if (!isValueReceivedOnFaceExpired(face) && face_value.map_requested) {
      // A map is requested in this face.
      blink::state::SetMapRequestedFace(face);

      return;
    }
  }
}

void Setup() {
  broadcast::message::handler::Set(
      {MESSAGE_EXTERNAL_PROPAGATE_COORDINATES, consume});
}

void Process() { maybe_propagate(); }

void __attribute__((noinline)) StartMapping() {
  // We are the mapping origin. Add ourselves to the map.
  add_local_to_map();

  propagation_timer_.set(GAME_MAP_PROPAGATION_TIMEOUT);
}

bool GetMapping() { return (!propagation_timer_.isExpired()); }

void ComputeMapStats() {
  memset(&stats_, 0, sizeof(Statistics));

  for (byte i = 0; i < index_; ++i) {
    const Data& map_data = map_[i];
    // Update number of players.
    if (map_data.player != 0 &&
        stats_.player[map_data.player].blink_count == 0) {
      stats_.player_count++;
    }

    // Update player blink count.
    stats_.player[map_data.player].blink_count++;

    // Update player can move.
    for (byte j = 0; j < index_; ++j) {
      if ((map_[j].player == 0) &&
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
          ((map_data.player == 0) &&
           (position::Distance(position::Coordinates{
                (int8_t)map_data.x, (int8_t)map_data.y}) <= 2));
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

position::Coordinates GetMoveOrigin() { return move_data_.origin; }

void SetMoveTarget(int8_t x, int8_t y) {
  set_data(x, y, &move_data_.target, blink::state::SetTarget);
}

position::Coordinates GetMoveTarget() { return move_data_.target; }

void __attribute__((noinline)) CommitMove() {
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

bool __attribute__((noinline)) ValidState() {
  return (stats_.player[0].blink_count > 0) && (stats_.player_count > 1);
}

bool MaybeUpload() {
  update_map_requested_face();

  byte face = blink::state::GetMapRequestedFace();

  if ((face == FACE_COUNT) || Uploaded() ||
      (game::state::Get() != GAME_STATE_PLAY)) {
    // Only send a map when we are sure we have one.
    return false;
  }

  switch (upload_state_) {
    case GAME_MAP_UPLOAD_STATE_SEND_METADATA: {
      // Upload just started. Send map metadata.
      byte payload[2] = {index_, game::state::GetData()};
      if (sendDatagramOnFace(payload, 2, face)) {
        // Size sent. Switch to actual map upload.
        upload_state_ = GAME_MAP_UPLOAD_STATE_UPLOAD;
      }
      break;
    }
    case GAME_MAP_UPLOAD_STATE_UPLOAD:
      // Now upload the actual map in chunks of
      // GAME_MAP_UPLOAD_MAX_CHUNK_SIZE.
      byte remaining = index_ - upload_index_;
      byte delta = remaining > GAME_MAP_UPLOAD_MAX_CHUNK_SIZE
                       ? GAME_MAP_UPLOAD_MAX_CHUNK_SIZE
                       : remaining;
      if (sendDatagramOnFace(&(map_[upload_index_]), delta, face)) {
        // CHunk sent. Increase the map upload index.
        upload_index_ += delta;
      }
      break;
  }

  return true;
}

bool Uploaded() { return ((index_ > 0) && (upload_index_ == index_)); }

void Reset() {
  index_ = 0;
  propagation_index_ = 0;
  upload_index_ = 0;
  upload_state_ = GAME_MAP_UPLOAD_STATE_SEND_METADATA;

  ComputeMapStats();
}

}  // namespace map

}  // namespace game
