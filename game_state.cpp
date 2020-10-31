
#include "game_state.h"

#include "game_message.h"
#include "game_player.h"

namespace game {

struct State {
  byte current;
  byte previous;
  byte current_specific;
  byte previous_specific;
  byte player;
  bool from_network;
};
static State state_;

namespace state {

static byte blink_count_[GAME_PLAYER_MAX_PLAYERS + 1];

void Set(byte state, bool from_network) {
  state_.previous = state_.current;
  state_.current = state;

  state_.from_network = from_network;
}

byte Get() { return state_.current; }

void SetSpecific(byte specific_state, bool from_network) {
  state_.previous_specific = state_.current_specific;
  state_.current_specific = specific_state;

  state_.from_network = from_network;
}

byte GetSpecific() { return state_.current_specific; }

void __attribute__((noinline)) SetPlayer(byte player) {
  state_.player = player;
}

byte __attribute__((noinline)) GetPlayer() { return state_.player; }

void NextPlayer() {
  byte current_player = GetPlayer();

  byte next_player = game::player::GetNext(current_player);
  while ((blink_count_[next_player] == 0 || next_player == 0) &&
         next_player != current_player) {
    next_player = game::player::GetNext(next_player);
  }

  SetPlayer(next_player);
}

#define UPDATE_BOARD_STATE_UPDATE 0
#define UPDATE_BOARD_STATE_CHECKING 1
#define UPDATE_BOARD_STATE_SEND_FLASH 2
#define UPDATE_BOARD_STATE_REPORT 3
#define UPDATE_BOARD_STATE_DONE 4

static byte update_board_state_state_ = UPDATE_BOARD_STATE_UPDATE;
static bool update_board_state_error_;

byte UpdateBoardState() {
  switch (update_board_state_state_) {
    case UPDATE_BOARD_STATE_UPDATE: {
      broadcast::Message reply;
      if (game::message::SendCheckBoardState(&reply)) {
        SetBlinkCount(reply.payload);

        update_board_state_state_ = UPDATE_BOARD_STATE_CHECKING;
      }

      break;
    }
    case UPDATE_BOARD_STATE_CHECKING: {
      update_board_state_state_ = UPDATE_BOARD_STATE_SEND_FLASH;
      update_board_state_error_ = true;

      byte* blink_count = GetBlinkCount();
      if (blink_count[0] > 0) {
        // We have at least one empty Blink.

        // Check player blinks.
        byte players_count = 0;
        for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
          if (blink_count[i] > 0) players_count++;
        }

        if (players_count >= 2) {
          // We have at least 2 players. Board state is valid.
          update_board_state_state_ = UPDATE_BOARD_STATE_REPORT;
          update_board_state_error_ = false;
        }
      }

      break;
    }
    case UPDATE_BOARD_STATE_SEND_FLASH:
      if (game::message::SendFlash()) {
        update_board_state_state_ = UPDATE_BOARD_STATE_REPORT;
      }

      break;
    case UPDATE_BOARD_STATE_REPORT:
      if (game::message::SendReportBoardState()) {
        update_board_state_state_ = UPDATE_BOARD_STATE_DONE;
      }

      break;
    case UPDATE_BOARD_STATE_DONE:
      if (isDatagramPendingOnAnyFace()) {
        break;
      }

      update_board_state_state_ = UPDATE_BOARD_STATE_UPDATE;

      if (update_board_state_error_) {
        return GAME_STATE_UPDATE_BOARD_STATE_ERROR;
      }

      return GAME_STATE_UPDATE_BOARD_STATE_OK;
  }

  return GAME_STATE_UPDATE_BOARD_STATE_UPDATING;
}

void __attribute__((noinline)) SetBlinkCount(byte* blink_count) {
  for (byte i = 0; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
    blink_count_[i] = blink_count == nullptr ? 0 : blink_count[i];
  }
}

byte* __attribute__((noinline)) GetBlinkCount() { return blink_count_; }

void Reset() {
  state_.current = GAME_STATE_IDLE;
  state_.previous = GAME_STATE_IDLE;
  state_.current_specific = 0;
  state_.previous_specific = 0;
  state_.player = 0;
  state_.from_network = false;

  SetBlinkCount(nullptr);
}

bool Changed(bool include_specific) {
  return include_specific
             ? state_.current != state_.previous ||
                   state_.current_specific != state_.previous_specific
             : state_.current != state_.previous;
}

bool Propagate() {
  if (!Changed() || state_.from_network) return true;

  game::message::GameStateChangeData data;

  data.state = state_.current;
  data.specific_state = state_.current_specific;
  data.next_player = state_.player - 1;

  if (!game::message::SendGameStateChange(data.value)) {
    return false;
  }

  return true;
}

bool FromNetwork() { return state_.from_network; }

}  // namespace state

}  // namespace game
