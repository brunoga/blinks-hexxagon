#include "game_state_end.h"

#include "blink_state.h"
#include "game_player.h"
#include "game_state.h"

namespace game {

namespace state {

namespace end {

void Handler(bool state_changed, byte* state, byte* specific_state) {
  if (state_changed) {
    byte max_count = 0;
    byte num_players_max_count = 0;

    byte players_max_count[GAME_PLAYER_MAX_PLAYERS];

    for (byte i = 1; i < GAME_PLAYER_MAX_PLAYERS + 1; ++i) {
      byte player_count = game::state::GetBlinkCount(i);
      if (player_count >= max_count) {
        if (player_count > max_count) {
          max_count = player_count;
          num_players_max_count = 1;
        } else {
          num_players_max_count++;
        }

        players_max_count[num_players_max_count - 1] = i;
      }
    }

    blink::state::SetPlayer(
        players_max_count[random(num_players_max_count - 1)]);
  }

  if (buttonSingleClicked()) {
    *state = GAME_STATE_IDLE;
    *specific_state = 0;
  }
}  // namespace end

}  // namespace end

}  // namespace state

}  // namespace game