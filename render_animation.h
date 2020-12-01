#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

namespace render {

namespace animation {

void ResetPulseTimer();

void Pulse(const Color& base_color, byte start, byte slowdown);
void Spinner(const Color& spinner_color, byte num_faces, byte slowdown);

bool Explosion(const Color& base_color);
bool Lightning(byte origin_face);

bool TakeOver(const Color& base_color, byte origin_face);

}  // namespace animation

}  // namespace render

#endif