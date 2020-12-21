#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

namespace render {

namespace animation {

void ResetTimer();

void Pulse(const Color& base_color, byte start, byte slowdown);
void Spinner(const Color& spinner_color, byte slowdown);

bool Explosion(Color base_color);

}  // namespace animation

}  // namespace render

#endif