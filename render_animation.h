#ifndef RENDER_ANIMATION_H_
#define RENDER_ANIMATION_H_

#include "blinklib.h"

// Disabling the lightning animation saves a lot of space that can be used for
// other features. We will keep it disabled for now (but if I ever find the
// space to reenable it, I will, as it looks cool).
#define RENDER_ANIMATION_TAKE_OVER_DISABLE_LIGHTNING

namespace render {

namespace animation {

void ResetPulseTimer();

void Pulse(const Color& base_color, byte start, byte slowdown);
void Spinner(const Color& spinner_color, byte num_faces, byte slowdown);

#ifndef RENDER_ANIMATION_TAKE_OVER_DISABLE_LIGHTNING
bool TakeOver(const Color& base_color, byte origin_face);
#else
bool TakeOver(const Color& base_color);
#endif

}  // namespace animation

}  // namespace render

#endif