#include "game_state_play.h"

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "message.h"
#include "util.h"

#define NEIGHBOR_TYPE_TARGET 0
#define NEIGHBOR_TYPE_ENEMY 1

namespace game {

namespace state {

namespace play {

static bool auto_select_ = false;

static bool explosion_started_ = false;

static bool __attribute__((noinline)) search_neighbor_type(byte neighbor_type) {
  FOREACH_FACE(f) {
    blink::state::FaceValue face_value;
    face_value.value = getLastValueReceivedOnFace(f);

    if ((neighbor_type == NEIGHBOR_TYPE_TARGET) && face_value.target) {
      return true;
    }

    if ((neighbor_type == NEIGHBOR_TYPE_ENEMY) && (face_value.player != 0) &&
        (face_value.player != blink::state::GetPlayer())) {
      return true;
    }
  }

  return false;
}

static bool do_explosion(byte explode_to_player) {
  if (!blink::state::GetExploding()) {
    if (!explosion_started_) {
      blink::state::SetExploding(true);
      explosion_started_ = true;
    } else {
      blink::state::SetPlayer(explode_to_player);
      explosion_started_ = false;
      return true;
    }
  }

  return false;
}

static void select_origin(byte* specific_state) {
  // We are going to select an origin, so reset any blink that is currently one.
  blink::state::SetOrigin(false);

  // Also, if there is a target selected. It must be reset (as we have no
  // origin).
  blink::state::SetTarget(false);

  // Also reset any potential targets.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  // This blink belongs to a player, but not the current one. Nothing to do.
  if (blink::state::GetPlayer() != game::state::GetPlayer()) return;

  // We pass all checks, but we do nothing until we get a click.
  if (!util::NoSleepButtonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  blink::state::SetOrigin(true);

  // Indicate that an origin was selected.
  *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
}

static void origin_selected(byte* specific_state) {
  // Only the origin blink has anything to do here.
  if (!blink::state::GetOrigin()) return;

  // Look for possible targets. Only continue when all other blinks report in.
  broadcast::Message reply;
  if (!game::message::SendGameStatePlayFindTargets(&reply)) return;

  // Did we find any targets?
  if (reply.payload[0] == 0) {
    // Nope. Notify the player and reset to selecting an origin (hoppefully a
    // different one).
    //
    // It is ok to not check the return value here as we know we just received
    // the reply for the find targets message so there can not be any message
    // pending to be delivered and send will succeed.
    game::message::SendFlash();

    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We have at least one possible target.
  *specific_state = GAME_STATE_PLAY_SELECT_TARGET;
}

static void select_target(byte* specific_state) {
  // We are going to select a target, so reset any blink that is currently one.
  blink::state::SetTarget(false);

  // If we are not a possible target or a Blink that belongs to the current
  // player, then there is also nothing to do.
  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE &&
      blink::state::GetPlayer() != game::state::GetPlayer()) {
    return;
  }

  // We pass all checks, but we do nothing until we get a click or auto
  // selection is enabled.
  if (!util::NoSleepButtonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Are we a blink that belongs to the current player?
  if (blink::state::GetPlayer() == game::state::GetPlayer()) {
    if (!blink::state::GetOrigin()) {
      // If we are not the origin we automatically switch to the new origin. If
      // we are the current origin, then we just deselect ourselves.
      auto_select_ = true;
    }

    // Change our state accordingly.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We are a valid target.
  blink::state::SetTarget(true);

  *specific_state = GAME_STATE_PLAY_TARGET_SELECTED;
}

static void target_selected(byte* specific_state) {
  if (!blink::state::GetTarget() &&
      blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_TARGET &&
      blink::state::GetPlayer() != game::state::GetPlayer()) {
    // We are not the currently selected target, an alternate
    // target or a Blink that belongs to the current player.
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!util::NoSleepButtonSingleClicked()) return;

  // Button was clicked and we are the selected target. Confirmn move.
  if (blink::state::GetTarget()) {
    *specific_state = GAME_STATE_PLAY_CONFIRM_MOVE;

    return;
  }

  // Origin was clicked. Deselect it.
  if (blink::state::GetOrigin()) {
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // Another target was clicked. Select it.
  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_TARGET) {
    auto_select_ = true;

    *specific_state = GAME_STATE_PLAY_SELECT_TARGET;

    return;
  }

  // Another origin was clicked. Select it.
  if (blink::state::GetPlayer() == game::state::GetPlayer()) {
    auto_select_ = true;

    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
  }
}

static void confirm_move(byte* specific_state) {
  if (search_neighbor_type(NEIGHBOR_TYPE_TARGET)) {
    if (blink::state::GetPlayer() != 0 &&
        blink::state::GetPlayer() != game::state::GetPlayer()) {
      // We are being conquered, trigger explosion animation.
      do_explosion(game::state::GetPlayer());
    }
  } else {
    if (blink::state::GetOrigin()) {
      // We are the origin and the target is not an immediate neighboor. We are
      // moving from here so reset ourselves.
      blink::state::Reset();
    }
  }

  // Reset origin so the spinning animation stops.
  blink::state::SetOrigin(false);

  // Clear target type for everybody.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  if (!blink::state::GetTarget()) return;

  // We are the target, so we are now owned by the current player.
  blink::state::SetPlayer(game::state::GetPlayer());

  if (search_neighbor_type(NEIGHBOR_TYPE_ENEMY)) return;

  *specific_state = GAME_STATE_PLAY_MOVE_CONFIRMED;
}

static void move_confirmed(byte* state, byte* specific_state) {
  if (!blink::state::GetTarget()) return;

  byte result = game::state::UpdateBoardState();

  if (result == GAME_STATE_UPDATE_BOARD_STATE_UPDATING) return;

  if (result == GAME_STATE_UPDATE_BOARD_STATE_ERROR) {
    // Board is in a state where the game can not continue. The end.
    *state = GAME_STATE_END;
    *specific_state = 0;

    return;
  }

  // Move to next turn.
  game::state::NextPlayer();

  *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  (void)state_changed;

  // Check for turn passing.
  if (buttonDoubleClicked()) {
    *specific_state = GAME_STATE_PLAY_PASS_TURN;

    game::state::NextPlayer();

    return;
  }

  switch (*specific_state) {
    case GAME_STATE_PLAY_SELECT_ORIGIN:
      select_origin(specific_state);
      break;
    case GAME_STATE_PLAY_ORIGIN_SELECTED:
      origin_selected(specific_state);
      break;
    case GAME_STATE_PLAY_SELECT_TARGET:
      select_target(specific_state);
      break;
    case GAME_STATE_PLAY_TARGET_SELECTED:
      target_selected(specific_state);
      break;
    case GAME_STATE_PLAY_CONFIRM_MOVE:
      confirm_move(specific_state);
      break;
    case GAME_STATE_PLAY_MOVE_CONFIRMED:
      move_confirmed(state, specific_state);
      break;
    case GAME_STATE_PLAY_PASS_TURN:
      game::state::SetSpecific(GAME_STATE_PLAY_SELECT_ORIGIN, true);
      *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
