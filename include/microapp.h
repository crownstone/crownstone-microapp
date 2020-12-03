#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint8_t MAX_PAYLOAD = 32;

/*
 * The structure used for communication.
 */
typedef struct {
	uint8_t payload[MAX_PAYLOAD];
	int length;
} message_t;

/*
 * To save space have a single global message object.
 */
extern message_t global_msg;

/*
 * Send a message to the bluenet code. This is the function that is called - in the end - by all the functions
 * that have to reach the microapp code.
 */
int sendMessage(message_t msg);

#ifdef __cplusplus
}
#endif
