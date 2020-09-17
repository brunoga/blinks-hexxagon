#include "game_state_play.h"

#include "blink_state.h"
#include "game_message.h"
#include "game_state.h"
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
    face_value.as_byte = getLastValueReceivedOnFace(f);

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
  if (!blink::state::GetAnimating()) {
    if (!explosion_started_) {
      blink::state::SetAnimating(true);
      explosion_started_ = true;
    } else {
      blink::state::SetPlayer(explode_to_player);
      explosion_started_ = false;
      return true;
    }
  }

  return false;
}

static void select_origin(byte* state, byte* specific_state) {
  (void)state;

  bool button_clicked = util::NoSleepButtonSingleClicked();

  // We are going to select an origin, so reset any Blink that is currently
  // one.
  blink::state::SetOrigin(false);

  // Also, if there is a target selected. It must be reset (as we have no
  // origin).
  blink::state::SetTarget(false);

  // Also reset any potential targets.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  // This blink belongs to a player, but not the current one. Nothing to do.
  if (blink::state::GetPlayer() != game::state::GetPlayer()) return;

  // We pass all checks, but we do nothing until we get a click.
  if (!button_clicked && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  blink::state::SetOrigin(true);

  // Indicate that an origin was selected.
  *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
}

static void origin_selected(byte* state, byte* specific_state) {
  (void)state;

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

static void select_target(byte* state, byte* specific_state) {
  (void)state;

  bool button_clicked = util::NoSleepButtonSingleClicked();

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
  if (!button_clicked && !auto_select_) return;

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

static void target_selected(byte* state, byte* specific_state) {
  (void)state;

  bool button_clicked = util::NoSleepButtonSingleClicked();

  if (!blink::state::GetTarget() &&
      blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_TARGET &&
      blink::state::GetPlayer() != game::state::GetPlayer()) {
    // We are not the currently selected target, an alternate
    // target or a Blink that belongs to the current player.
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!button_clicked) return;

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

static void confirm_move(byte* state, byte* specific_state) {
  (void)state;

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

  if (buttonDoubleClicked()) {
    // Turn is being passed. Switch to next player.
    game::state::NextPlayer();

    // Manually force a state that is not the select origin one so that even if
    // we are currently in it, the state will be propagate to the other Blinks
    // (the next player information is propagated with state changes). This is
    // cheaper than keeping track of the previous player in the game state and
    // using that for automatically deciding if the state should be propagated
    // or not.
    game::state::SetSpecific(GAME_STATE_PLAY_MOVE_CONFIRMED);

    // Next player will select origin.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  switch (*specific_state) {
    case GAME_STATE_PLAY_SELECT_ORIGIN:
      select_origin(state, specific_state);
      break;
    case GAME_STATE_PLAY_ORIGIN_SELECTED:
      origin_selected(state, specific_state);
      break;
    case GAME_STATE_PLAY_SELECT_TARGET:
      select_target(state, specific_state);
      break;
    case GAME_STATE_PLAY_TARGET_SELECTED:
      target_selected(state, specific_state);
      break;
    case GAME_STATE_PLAY_CONFIRM_MOVE:
      confirm_move(state, specific_state);
      break;
    case GAME_STATE_PLAY_MOVE_CONFIRMED:
      move_confirmed(state, specific_state);
      break;
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
