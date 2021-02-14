#ifndef BLINK_STATE_RENDER_H_
#define BLINK_STATE_RENDER_H_

#include "blinklib.h"

namespace blink {

namespace state {

namespace render {

void ResetPulseTimer();

void Pulse(const Color& base_color, byte start, byte slowdown);
void Pulse(void (*render_function)(byte dim_level), byte start, byte slowdown);

void Spinner(const Color& spinner_color, byte slowdown);

bool Explosion(Color base_color);

// Not an animation per see, but being here will do.
void Empty(byte dim_level);

}  // namespace render

}  // namespace state

}  // namespace blink

#endif