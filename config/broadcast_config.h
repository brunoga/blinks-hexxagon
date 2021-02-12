#ifndef BROADCAST_CONFIG_H_
#define BROADCAST_CONFIG_H_

// This is the maximum payload we will need.
#define BROADCAST_MESSAGE_PAYLOAD_BYTES 4

// Disable message replies. Saves considerable space.
#define BROADCAST_DISABLE_REPLIES

// Enable message handler support. Required by the mapping code.
#define BROADCAST_ENABLE_MESSAGE_HANDLER

#endif
