#ifndef UTIL_H_
#define UTIL_H_

#define NEIGHBOR_TYPE_TARGET 0
#define NEIGHBOR_TYPE_ENEMY 1
#define NEIGHBOR_TYPE_SELF_DESTRUCT 2

namespace util {

bool NoSleepButtonSingleClicked();
bool SearchNeighborType(byte neighbor_type, byte* source_face);

}  // namespace util

#endif