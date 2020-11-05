#ifndef RENDER_CONFIG_H_
#define RENDER_CONFIG_H_

#include <blinklib.h>

#include "blink_state.h"
#include "game_player.h"

// Idle state.
#define RENDER_CONFIG_IDLE_STATE_PULSE_START_DIM 143
#define RENDER_CONFIG_IDLE_STATE_PULSE_SLOWDOWN 10

// Setup state.
#define RENDER_CONFIG_SETUP_STATE_COLOR_DIM 127

// Play state.
#define RENDER_CONFIG_PLAY_STATE_SPINNER_COLOR WHITE
#define RENDER_CONFIG_PLAY_STATE_SPINNER_FACES 1
#define RENDER_CONFIG_PLAY_STATE_SPINNER_SLOWDOWN 50

#define RENDER_CONFIG_PLAY_STATE_PULSE_START_DIM 175
#define RENDER_CONFIG_PLAY_STATE_PULSE_SLOWDOWN 8

#define RENDER_CONFIG_PLAY_STATE_COLOR_DIM 159

// End state.
#define RENDER_CONFIG_END_STATE_PULSE_START_DIM 143
#define RENDER_CONFIG_END_STATE_PULSE_SLOWDOWN 2

#endif