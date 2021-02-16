#ifndef HEXXAGON_CONFIG_H_
#define HEXXAGON_CONFIG_H_

// Uncomment to enable debugging of data pending on faces. When enabled will
// turn of faces that have pending data on every loop iteration. If a face is
// constantly off, that is an indication that there is an issue (which might be
// a software bug but also a hardware one).
//#define HEXXAGON_ENABLE_PENDING_FACE_DEBUG

// Uncomment to switch reset from long-press to multi-click.
#define HEXXAGON_MULTI_CLICK_RESET

// Blink state render configuration (these are all non-optional).

// Idle state.
#define HEXXAGON_RENDER_IDLE_STATE_PULSE_START_DIM 143
#define HEXXAGON_RENDER_IDLE_STATE_PULSE_SLOWDOWN 10

// Setup state.
#define HEXXAGON_RENDER_SETUP_STATE_COLOR_DIM 127

#define HEXXAGON_RENDER_SETUP_STATE_PULSE_START_DIM 127
#define HEXXAGON_RENDER_SETUP_STATE_PULSE_SLOWDOWN 2

// Play state.
#define HEXXAGON_RENDER_PLAY_STATE_SPINNER_COLOR WHITE
#define HEXXAGON_RENDER_PLAY_STATE_SPINNER_SLOWDOWN 50

#define HEXXAGON_RENDER_PLAY_STATE_PULSE_START_DIM 32
#define HEXXAGON_RENDER_PLAY_STATE_PULSE_SLOWDOWN 4

#define HEXXAGON_RENDER_PLAY_STATE_COLOR_DIM 127
#define HEXXAGON_RENDER_PLAY_STATE_SELECT_TARGET_COLOR_DIM 63

// End state.
#define HEXXAGON_RENDER_END_STATE_PULSE_START_DIM 143
#define HEXXAGON_RENDER_END_STATE_PULSE_SLOWDOWN 2

#endif