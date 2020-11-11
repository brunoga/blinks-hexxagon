#include "blink_coordinates.h"

namespace blink {

namespace coordinates {

static int8_t x_coordinate_;
static int8_t y_coordinate_;
static int8_t z_coordinate_;

static int8_t compute_z_coordinate(int8_t x, int8_t y) { return -(x + y); }

void Set(int8_t x, int8_t y) {
  x_coordinate_ = x;
  y_coordinate_ = y;
  z_coordinate_ = compute_z_coordinate(x, y);
}

int8_t X() { return x_coordinate_; }
int8_t Y() { return y_coordinate_; }
int8_t Z() { return z_coordinate_; }

}  // namespace coordinates

}  // namespace blink
