#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

namespace render {

namespace animation {

void ResetPulseTimer();

void Pulse(const Color& base_color, byte start, byte slowdown);
void Pulse(void (*render_function)(byte dim_level), byte start, byte slowdown);

void Spinner(const Color& spinner_color, byte slowdown);

bool Explosion(Color base_color);

// Not an animation per see, but being here will do.
void Empty(byte dim_level);

}  // namespace animation

}  // namespace render

#endif