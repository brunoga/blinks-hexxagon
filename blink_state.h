#ifndef BLINK_STATE_H_
#define BLINK_STATE_H_

#include <blinklib.h>

#define BLINK_STATE_TARGET_TYPE_NONE 0
#define BLINK_STATE_TARGET_TYPE_TARGET 1

namespace blink {

namespace state {

union FaceValue {
  struct {
    byte unused : 3;         // Unused for now.
    bool self_destruct : 1;  // Is self-destruct in progress?
    bool target : 1;         // Is it the current target?
    byte player : 3;         // 0 means empty.
  };

  byte as_byte;
};

void SetOrigin(bool origin);
bool GetOrigin();

void SetTarget(bool target);
bool GetTarget();

void SetTargetType(byte target_type);
byte GetTargetType();

void SetPlayer(byte player);
byte GetPlayer();

void SetSelfDestruct(bool self_destruct);
byte GetSelfDestruct();

void SetAnimating(bool animating);
bool GetAnimating();
void SetAnimatingParam(void* animating_param);
void SetAnimatingFunction(bool (*animating_function)(void* param));
bool RunAnimatingFunction();

void StartColorOverride();
bool GetColorOverride();

void Reset();

void Render(byte game_state);

}  // namespace state

}  // namespace blink

#endif