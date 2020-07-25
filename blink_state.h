#ifndef BLINK_STATE_H_
#define BLINK_STATE_H_

#include <blinklib.h>

#define BLINK_STATE_TYPE_EMPTY 0
#define BLINK_STATE_TYPE_PLAYER 1

#define BLINK_STATE_TARGET_TYPE_NONE 0
#define BLINK_STATE_TARGET_TYPE_MOVE 1
#define BLINK_STATE_TARGET_TYPE_COPY 2

namespace blink {

namespace state {

void SetType(byte type);
byte GetType();

void SetOrigin(bool origin);
bool GetOrigin();

void SetTarget(bool target);
bool GetTarget();

void SetTargetType(byte target_type);
byte GetTargetType();

void SetPlayer(byte player);
byte GetPlayer();

void Reset();

void Render(byte game_state);

}  // namespace state

}  // namespace blink

#endif