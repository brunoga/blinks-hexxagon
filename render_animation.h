#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

namespace render {

namespace animation {

void Pulse(const Color& base_color, byte start, byte speed);
void WhiteSpinner(const Color& base_color);

bool Takeover(const Color& start, const Color& end);

}  // namespace animation

}  // namespace render

#endif