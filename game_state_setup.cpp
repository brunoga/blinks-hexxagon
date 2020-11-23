#include "game_state_setup.h"

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"
#include "util.h"

namespace game {

namespace state {

namespace setup {

static bool validating_;

static Timer wait_timer_;

static void players(byte* state, byte* specific_state) {
  (void)state;

  if (buttonDoubleClicked()) {
    // We seem to be done with setting up the game. We now need to validate
    // if the board state is actually valid.

    // We overload origin here to mean the coordinate system origin.
    blink::state::SetOrigin(true);

    *specific_state = GAME_STATE_SETUP_MAP;
  } else if (util::NoSleepButtonSingleClicked()) {
    // Blink was clicked. Switch it to next player. Note that we will never
    // reach this point if we are validating the game state until we finish
    // doing it. This is intentional and prevents the local state changing
    // while we are validating the board. Clicking other Blinks could
    // potentially mess things up, but there is only so much we can do with
    // the resources available so we ignore it.
    blink::state::SetPlayer(game::player::GetNext(blink::state::GetPlayer()));
  }
}

static void map(byte* state, byte* specific_state) {
  (void)state;

  // And start the mapping process.
  game::map::StartMapping(blink::state::GetOrigin());

  // When we return from mapping, we go straight to board validation.
  *specific_state = GAME_STATE_SETUP_VALIDATE;
}

static void validate(byte* state, byte* specific_state) {
  if (!blink::state::GetOrigin()) return;

  if (!validating_) {
    wait_timer_.set(2000);
    validating_ = true;
  }

  if (!wait_timer_.isExpired()) return;

  if (game::map::GetPlayerCount() >= 2 && game::map::GetBlinkCount(0) > 0) {
    // Game state is good. Switch to first available player.
    game::state::NextPlayer();

    blink::state::SetOrigin(false);
    validating_ = false;

    *state = GAME_STATE_PLAY;
    *specific_state = 0;

    return;
  }

  if (!game::message::SendFlash()) return;

  blink::state::SetOrigin(false);
  validating_ = false;

  *specific_state = GAME_STATE_SETUP_PLAYERS;
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    validating_ = false;
    *specific_state = GAME_STATE_SETUP_PLAYERS;
  }

  switch (*specific_state) {
    case GAME_STATE_SETUP_PLAYERS:
      players(state, specific_state);
      break;
    case GAME_STATE_SETUP_MAP:
      map(state, specific_state);
      break;
    case GAME_STATE_SETUP_VALIDATE:
      validate(state, specific_state);
      break;
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game