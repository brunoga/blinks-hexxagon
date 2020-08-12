#include "game_state_setup.h"

#include "blink_state.h"
#include "debug.h"
#include "game_message.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace setup {

bool checking_board_ = false;

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    blink::state::Reset();
  }

  if (buttonDoubleClicked() || checking_board_) {
    checking_board_ = true;

    broadcast::Message reply;
    if (!game::message::SendCheckBoard(&reply)) return;

    checking_board_ = false;

    LOGLN(reply.payload[0]);
    if (reply.payload[0] == 0) {
      // We need at least one empty Blink.

      // TODO(bga): Add some visual feedback to indicate something is wrong.
      return;
    }

    byte players_count = 0;
    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      LOGLN(reply.payload[i]);
      if (reply.payload[i] > 0) players_count++;
    }

    if (players_count < 2) {
      // We need at least two players.

      // TODO(bga): Add some visual feedback to indicate something is wrong.
      return;
    }

    game::state::SetBlinkCount(reply.payload);

    *state = GAME_STATE_PLAY;
    *specific_state = 0;
  }

  if (buttonSingleClicked()) {
    blink::state::SetPlayer(game::player::GetNext(blink::state::GetPlayer()));
  }
}

}  // namespace setup

}  // namespace state

}  // namespace game
