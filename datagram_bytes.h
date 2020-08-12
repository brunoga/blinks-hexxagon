#ifndef DATAGRAM_BYTES_H_
#define DATAGRAM_BYTES_H_

#include <blinklib.h>

#include "payload_bytes.h"

// Max number of datagram bytes in a message.
#define DATAGRAM_BYTES MESSAGE_PAYLOAD_BYTES + 1

#endif
