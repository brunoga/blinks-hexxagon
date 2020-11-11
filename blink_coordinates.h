#ifndef BLINKS_COORDINATES_H_
#define BLINKS_COORDINATES_H_

#include <blinklib.h>

namespace blink {

namespace coordinates {

void Set(int8_t x, int8_t y);

int8_t X();
int8_t Y();
int8_t Z();

}  // namespace coordinates

}  // namespace blink

#endif