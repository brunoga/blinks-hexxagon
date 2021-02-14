#include "game_state_play.h"

#include "blink_state.h"
#include "blink_state_render.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "src/blinks-position/position.h"
#include "util.h"

namespace game {

namespace state {

namespace play {

static bool auto_select_ = false;

static void select_origin(byte* state) {
  if (game::state::Changed()) {
    // State changed. Reset animation timer to improve synchronization.
    blink::state::render::ResetPulseTimer();
  }

  bool button_clicked = util::NoSleepButtonSingleClicked();

  // We are going to select an origin, so reset any Blink that is currently
  // one.
  blink::state::SetOrigin(false);

  // Also, if there is a target selected. It must be reset (as we have no
  // origin).
  blink::state::SetTarget(false);

  // This blink belongs to a player, but not the current one. Nothing to do.
  if (blink::state::GetPlayer() != game::state::GetPlayer()) return;

  if (!game::map::GetStatistics().local_blink_empty_space_in_range) {
    // We can not move.
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!button_clicked && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  game::map::SetMoveOrigin(position::Local());
  blink::state::SetOrigin(true);

  // Indicate that an origin was selected.
  *state = GAME_STATE_PLAY_ORIGIN_SELECTED;
}

static void origin_selected(byte* state) {
  // Only the origin blink has anything to do here.
  if (!blink::state::GetOrigin()) return;

  if (!game::message::SendSelectBlink(
          MESSAGE_SELECT_ORIGIN, position::Local().x, position::Local().y)) {
    return;
  }

  *state = GAME_STATE_PLAY_SELECT_TARGET;
}

static void select_target(byte* state) {
  bool button_clicked = util::NoSleepButtonSingleClicked();

  // We are going to select a target, so reset any blink that is currently
  // one.
  blink::state::SetTarget(false);

  if (blink::state::GetPlayer() == GAME_PLAYER_NO_PLAYER &&
      position::Distance(game::map::GetMoveOrigin()) <= 2) {
    blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_TARGET);
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!button_clicked) return;

  // Are we a blink that belongs to the current player?
  if (blink::state::GetPlayer() == game::state::GetPlayer() &&
      game::map::GetStatistics().local_blink_empty_space_in_range) {
    if (!blink::state::GetOrigin()) {
      // If we are not the origin we automatically switch to the new origin.
      // If we are the current origin, then we just deselect ourselves.
      auto_select_ = true;
    }

    // Change our state accordingly.
    *state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) return;

  // We are a valid target.
  game::map::SetMoveTarget(position::Local());
  blink::state::SetTarget(true);

  *state = GAME_STATE_PLAY_TARGET_SELECTED;
}

static void target_selected(byte* state) {
  // Only the target blink has anything to do here.
  if (!blink::state::GetTarget()) return;

  if (!game::message::SendSelectBlink(
          MESSAGE_SELECT_TARGET, position::Local().x, position::Local().y)) {
    return;
  }

  *state = GAME_STATE_PLAY_MOVE_CONFIRMED;
}

static void move_confirmed(
    byte* state, const blink::state::face::ValueHandler& face_value_handler) {
  if (blink::state::GetOrigin() &&
      position::coordinates::Distance(game::map::GetMoveOrigin(),
                                      game::map::GetMoveTarget()) != 1) {
    // We are the origin and the target is not an immediate neighboor. We
    // are moving from here so reset ourselves.
    blink::state::Reset();
  }

  if (!blink::state::GetTarget()) return;

  // We are the target, so we are now owned by the current player.
  blink::state::SetPlayer(game::state::GetPlayer());

  if (face_value_handler.EnemyNeighbor()) return;

  *state = GAME_STATE_PLAY_RESOLVE_MOVE;
}

static void resolve_move(byte* state) {
  game::map::CommitMove();

  if (!blink::state::GetTarget()) return;

  if (util::CheckValidateStateAndReport()) {
    // Move to next turn.
    game::state::NextPlayer();

    *state = GAME_STATE_PLAY_SELECT_ORIGIN;
  } else {
    // Game over.
    *state = GAME_STATE_END;
  }
}

void Handler(byte* state,
             const blink::state::face::ValueHandler& face_value_handler) {
  if (*state == GAME_STATE_PLAY) {
    *state = GAME_STATE_PLAY_SELECT_ORIGIN;
  }

  blink::state::SetTargetType(BLINK_STATE_TARGET_TYPE_NONE);

  switch (*state) {
    case GAME_STATE_PLAY_SELECT_ORIGIN:
      select_origin(state);
      break;
    case GAME_STATE_PLAY_ORIGIN_SELECTED:
      origin_selected(state);
      break;
    case GAME_STATE_PLAY_SELECT_TARGET:
      select_target(state);
      break;
    case GAME_STATE_PLAY_TARGET_SELECTED:
      target_selected(state);
      break;
    case GAME_STATE_PLAY_MOVE_CONFIRMED:
      move_confirmed(state, face_value_handler);
      break;
    case GAME_STATE_PLAY_RESOLVE_MOVE:
      resolve_move(state);
      break;
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
