#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

namespace render {

namespace animation {

void Pulse(const Color& base_color, byte start, byte speed);
void Spinner(const Color& base_color, const Color& spinner_color);

bool Explosion(const Color& base_color);

}  // namespace animation

}  // namespace render

#endif