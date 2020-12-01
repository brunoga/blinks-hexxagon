#include "game_state_play.h"

#include "blink_state.h"
#include "game_map.h"
#include "game_message.h"
#include "game_state.h"
#include "render_animation.h"
#include "src/blinks-position/position.h"
#include "util.h"

#define NEIGHBOR_TYPE_TARGET 0
#define NEIGHBOR_TYPE_ENEMY 1

namespace game {

namespace state {

namespace play {

static bool auto_select_ = false;

static bool take_over_started_ = false;

static bool search_neighbor_type(byte neighbor_type, byte* source_face) {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      blink::state::FaceValue face_value;
      face_value.as_byte = getLastValueReceivedOnFace(f);

      *source_face = f;

      switch (neighbor_type) {
        case NEIGHBOR_TYPE_TARGET:
          if (face_value.target) {
            return true;
          }
          break;
        case NEIGHBOR_TYPE_ENEMY:
          if ((face_value.player != 0) &&
              (face_value.player != blink::state::GetPlayer())) {
            return true;
          }
          break;
      }
    }
  }

  return false;
}

static bool do_takeover(byte takeover_player, byte source_face) {
  if (!blink::state::GetTakeOver()) {
    if (!take_over_started_) {
#ifndef RENDER_ANIMATION_TAKE_OVER_DISABLE_LIGHTNING
      blink::state::SetTakeOverFace(source_face);
#else
      (void)source_face;
#endif
      blink::state::SetTakeOver(true);

      take_over_started_ = true;
    } else {
      blink::state::SetPlayer(takeover_player);

      take_over_started_ = false;

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

  if (!game::map::GetStats().local_blink_empty_space_in_range) {
    // We can not move.
    return;
  }

  // We pass all checks, but we do nothing until we get a click.
  if (!button_clicked && !auto_select_) return;

  auto_select_ = false;

  // Ok, we are now the origin.
  game::map::SetMoveOrigin(position::Local().x, position::Local().y);

  // Indicate that an origin was selected.
  *specific_state = GAME_STATE_PLAY_ORIGIN_SELECTED;
}

static void origin_selected(byte* state, byte* specific_state) {
  (void)state;

  // Only the origin blink has anything to do here.
  if (!blink::state::GetOrigin()) return;

  if (!game::message::SendSelectOrigin(position::Local().x,
                                       position::Local().y)) {
    return;
  }

  *specific_state = GAME_STATE_PLAY_SELECT_TARGET;
}

static void select_target(byte* state, byte* specific_state) {
  (void)state;

  bool button_clicked = util::NoSleepButtonSingleClicked();

  // We are going to select a target, so reset any blink that is currently
  // one.
  blink::state::SetTarget(false);

  // We pass all checks, but we do nothing until we get a click or auto
  // selection is enabled.
  if (!button_clicked && !auto_select_) return;

  auto_select_ = false;

  // Are we a blink that belongs to the current player?
  if (blink::state::GetPlayer() == game::state::GetPlayer() &&
      game::map::GetStats().local_blink_empty_space_in_range) {
    if (!blink::state::GetOrigin()) {
      // If we are not the origin we automatically switch to the new origin.
      // If we are the current origin, then we just deselect ourselves.
      auto_select_ = true;
    }

    // Change our state accordingly.
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;

    return;
  }

  if (blink::state::GetTargetType() == BLINK_STATE_TARGET_TYPE_NONE) return;

  // We are a valid target.
  game::map::SetMoveTarget(position::Local().x, position::Local().y);

  *specific_state = GAME_STATE_PLAY_TARGET_SELECTED;
}

static void target_selected(byte* state, byte* specific_state) {
  (void)state;

  // Only the target blink has anything to do here.
  if (!blink::state::GetTarget()) return;

  if (!game::message::SendSelectTarget(position::Local().x,
                                       position::Local().y)) {
    return;
  }

  *specific_state = GAME_STATE_PLAY_CONFIRM_MOVE;
}

static void confirm_move(byte* state, byte* specific_state) {
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

  // Button was clicked and we are the selected target. Move confirmed.
  if (blink::state::GetTarget()) {
    *specific_state = GAME_STATE_PLAY_MOVE_CONFIRMED;

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

static void move_confirmed(byte* state, byte* specific_state) {
  (void)state;

  byte source_face;
  if (search_neighbor_type(NEIGHBOR_TYPE_TARGET, &source_face)) {
    if (blink::state::GetPlayer() != 0 &&
        blink::state::GetPlayer() != game::state::GetPlayer()) {
      // We are being conquered, trigger explosion animation.
      if (!do_takeover(game::state::GetPlayer(), source_face)) return;
    }
  } else {
    if (blink::state::GetOrigin()) {
      // We are the origin and the target is not an immediate neighboor. We
      // are moving from here so reset ourselves.
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

  if (search_neighbor_type(NEIGHBOR_TYPE_ENEMY, &source_face)) return;

  *specific_state = GAME_STATE_PLAY_RESOLVE_MOVE;
}

static void resolve_move(byte* state, byte* specific_state) {
  game::map::CommitMove();

  if (!blink::state::GetTarget()) return;

  bool valid;
  if (util::CheckValidateStateAndReport(&valid)) {
    if (valid) {
      // Move to next turn.
      game::state::NextPlayer();

      *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
    } else {
      // Game over.
      *state = GAME_STATE_END;
    }
  }
}

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    *specific_state = GAME_STATE_PLAY_SELECT_ORIGIN;
  }

  if (buttonDoubleClicked()) {
    // Turn is being passed. Switch to next player.
    game::state::NextPlayer();

    // Manually force a state that is not the select origin one so that even
    // if we are currently in it, the state will be propagate to the other
    // Blinks (the next player information is propagated with state changes).
    // This is cheaper than keeping track of the previous player in the game
    // state and using that for automatically deciding if the state should be
    // propagated or not.
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
    case GAME_STATE_PLAY_RESOLVE_MOVE:
      resolve_move(state, specific_state);
      break;
  }
}

}  // namespace play

}  // namespace state

}  // namespace game
