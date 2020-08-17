#include "game_state_play.h"

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_state.h"
#include "message.h"

namespace game {

namespace state {

namespace play {

static bool auto_select_ = false;

static void select_origin(byte* state, byte* specific_state) {
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
  if (!buttonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  blink::state::SetOrigin(true);

  // Indicate that an origin was selected. This will be automatically propagated
  // to all blinks.
  *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;

  (void)state;
}

static void origin_selected(byte* state, byte* specific_state) {
  // Only the origin blink has anything to do here.
  if (!blink::state::GetOrigin()) return;

  // Look for possible targets. Only continue when all other blinks report in.
  broadcast::Message reply;
  if (!game::message::SendGameStatePlayFindTargets(&reply)) return;

  // Did we find any targets?
  if (reply.payload[0] == 0) {
    // Nope. Reset to selecting an origin (hoppefully a different one).
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We have at least one possible target.
  *specific_state = GAME_STATE_PLAY_SELECT_TARGET;

  (void)state;
}

static void select_target(byte* state, byte* specific_state) {
  // We are going to select a target, so reset any blink that is currently one.
  blink::state::SetTarget(false);

  // If this blink is the origin, there is nothing for it to do at this stage.
  if (blink::state::GetOrigin()) return;

  // If we are not a possible target or a blink that belongs to the current
  // player, then there is also nothing to do.
  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE &&
      blink::state::GetPlayer() != game::state::GetPlayer()) {
    return;
  }

  // We pass all checks, but we do nothing until we get a click or auto
  // selection is enabled for this blink.
  if (!buttonSingleClicked() && !auto_select_) return;

  auto_select_ = false;

  // Are we a blink that belongs to the current player?
  if (blink::state::GetPlayer() == game::state::GetPlayer()) {
    auto_select_ = true;

    // Change our state accordingly.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  // We are a valid target.
  blink::state::SetTarget(true);

  *specific_state = GAME_STATE_PLAY_TARGET_SELECTED;

  (void)state;
}

static void target_selected(byte* state, byte* specific_state) {
  if (!blink::state::GetTarget() && !blink::state::GetOrigin() &&
      blink::state::GetTargetType() != BLINK_STATE_TARGET_TYPE_TARGET &&
      blink::state::GetPlayer() != game::state::GetPlayer()) {
    // We are not the currently selected target, the origin, an alternate
    // target or an alternate origin. Nothing to do.
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!buttonSingleClicked()) return;

  // Button was clicked and we are the selected target. Confirmn move.
  if (blink::state::GetTarget()) {
    blink::state::SetArbitrator(true);

    *specific_state = GAME_STATE_PLAY_CONFIRM_MOVE;

    return;
  }

  // Origin was clicked. Deselect current selected target.
  if (blink::state::GetOrigin()) {
    *specific_state = GAME_STATE_PLAY_SELECT_TARGET;

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

  (void)state;
}

static bool neighboor_target() {
  FOREACH_FACE(f) {
    blink::state::FaceValue face_value;
    face_value.value = getLastValueReceivedOnFace(f);

    if (face_value.target) return true;
  }

  return false;
}

static void confirm_move(byte* state, byte* specific_state) {
  bool neighboor_is_target = neighboor_target();

  if (neighboor_is_target) {
    if (blink::state::GetPlayer() != 0 &&
        blink::state::GetPlayer() != game::state::GetPlayer()) {
      // Target is our neighboor and it is a different player from ourselves. We
      // now become that player too.
      blink::state::SetPlayer(game::state::GetPlayer());
    }
  } else {
    if (blink::state::GetOrigin()) {
      // We are the origin and the target is not an immediate neighboor. We are
      // moving from here so reset ourselves.
      blink::state::Reset();
    }
  }

  // Clear target type for everybody.
  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  if (!blink::state::GetArbitrator()) return;

  *specific_state = GAME_STATE_PLAY_MOVE_CONFIRMED;

  (void)state;
}

static void move_confirmed(byte* state, byte* specific_state) {
  // Now it is ok to clear the target flag.
  if (blink::state::GetTarget()) {
    // We are the target. Become a player blink.
    //
    // TODO(bga): We do not remove the target flags in confirm_move above so it
    // will still be present when we are reading the face value in a origin
    // neighboor blink. There is most likelly a better way to do this.
    blink::state::SetPlayer(game::state::GetPlayer());
    blink::state::SetTarget(false);
  }

  if (!blink::state::GetArbitrator()) return;

  byte result = game::state::UpdateBoardState();

  if (result == GAME_STATE_UPDATE_BOARD_STATE_UPDATING) return;

  if (result == GAME_STATE_UPDATE_BOARD_STATE_ERROR) {
    // Board is in a state where the game can not continue. The end.
    *state = GAME_STATE_END;
    *specific_state = 0;

    return;
  }

  blink::state::SetArbitrator(false);

  // Move to next turn.
  game::state::NextPlayer();

  *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    *state = GAME_STATE_PLAY;
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    game::state::SetPlayer(1);

    return;
  }

  if (*specific_state == GAME_STATE_PLAY_PASS_TURN) {
    game::state::SetSpecific(GAME_STATE_PLAY_SELECT_ORIGIN, true);
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
  }

  // Check for turn passing.
  if (buttonDoubleClicked()) {
    *specific_state = GAME_STATE_PLAY_PASS_TURN;

    game::state::NextPlayer();

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
