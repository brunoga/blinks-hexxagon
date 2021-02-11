#ifndef BLINK_STATE_H_
#define BLINK_STATE_H_

#include <blinklib.h>

#include "render_animation.h"

#define BLINK_STATE_TARGET_TYPE_NONE 0
#define BLINK_STATE_TARGET_TYPE_TARGET 1

namespace blink {

namespace state {

void SetOrigin(bool origin);
bool GetOrigin();

void SetTarget(bool target);
bool GetTarget();

void SetTargetType(byte target_type);
byte GetTargetType();

void SetPlayer(byte player);
byte GetPlayer();

void StartColorOverride();
bool GetColorOverride();

void Reset();

void Render(byte game_state);

}  // namespace state

}  // namespace blink

#endif