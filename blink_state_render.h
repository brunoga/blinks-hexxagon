#ifndef BLINK_STATE_RENDER_H_
#define BLINK_STATE_RENDER_H_

#include "blinklib.h"

namespace blink {

namespace state {

namespace render {

void ResetPulseTimer();

void Pulse(byte start, byte slowdown);

void Spinner(const Color& spinner_color, byte slowdown);

bool Explosion(Color base_color);

void Player(byte dim_level);

}  // namespace render

}  // namespace state

}  // namespace blink

#endif