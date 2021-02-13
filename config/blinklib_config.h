#ifndef BLINKLIB_CONFIG_H_
#define BLINKLIB_CONFIG_H_

// This is the maximum size of datagram we will use. Saves a considerable amount
// of memory.
#define BGA_CUSTOM_BLINKLIB_IR_DATAGRAM_LEN 5

// Set data transmission timeouts. Due to the number of datagrams being sent in
// multiple faces, the default values are not enough fot Hexxagon, so we
// increase them by 100 ms here.
#define BGA_CUSTOM_BLINKLIB_SEND_PROBE_TIMEOUT_MS 300
#define BGA_CUSTOM_BLINKLIB_FACE_EXPIRATION_TIMEOUT_MS 350

#endif