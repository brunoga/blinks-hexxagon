#include "game_state_setup.h"

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"
#include "src/blinks-orientation/orientation.h"
#include "util.h"

namespace game {

namespace state {

namespace setup {

static void players(byte* state, byte* specific_state) {
  (void)state;

  // Make sure map is empty.
  //
  // TODO(bga): There might be a race condition between the state change message
  // and the propagation of the map coordinates. If somehow a blink receive a
  // cordinate and only aftyer that gets the state change, the map will be reset
  // and the added Blink will be removed. Investigate.
  game::map::Reset();

  if (buttonDoubleClicked()) {
    // We seem to be done with setting up the game. We now need to validate
    // if the board state is actually valid.

    // We overload origin here to mean the coordinate system origin.
    blink::state::SetOrigin(true);

    // Reset our position and orientation, making us the origin of the
    // coordinate system.
    orientation::Reset();
    position::Reset();

    game::map::StartMapping();

    *specific_state = GAME_STATE_SETUP_MAP;
  } else if (util::NoSleepButtonSingleClicked()) {
    // Blink was clicked. Switch it to next player.
    blink::state::SetPlayer(game::player::GetNext(blink::state::GetPlayer()));
  }
}

static void map(byte* state, byte* specific_state) {
  (void)state;

  game::map::Process();

  if (game::map::GetMapping() || !blink::state::GetOrigin()) return;

  *specific_state = GAME_STATE_SETUP_VALIDATE;
}

static void validate(byte* state, byte* specific_state) {
  game::map::ComputeMapStats();

  if (!blink::state::GetOrigin()) return;

  bool valid;
  if (util::CheckValidateStateAndReport(&valid)) {
    if (valid) {
      // Game state is good. Switch to first available player.
      game::state::NextPlayer();

      *state = GAME_STATE_PLAY;
      *specific_state = 0;
    } else {
      *specific_state = GAME_STATE_SETUP_SELECT_PLAYERS;
    }

    blink::state::SetOrigin(false);
  }
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    *specific_state = GAME_STATE_SETUP_SELECT_PLAYERS;
  }

  switch (*specific_state) {
    case GAME_STATE_SETUP_SELECT_PLAYERS:
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