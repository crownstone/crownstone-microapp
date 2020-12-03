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


enum CommandMicroapp {
	CS_MICROAPP_COMMAND_LOG          = 0x01,
	CS_MICROAPP_COMMAND_DELAY        = 0x02,
	CS_MICROAPP_COMMAND_PIN          = 0x03,
	CS_MICROAPP_COMMAND_SERVICE_DATA = 0x04,
};

/*
 * Struct to set and read pins. This can be used for analog and digital writes and reads. For digital writes it is
 * just zeros or ones. For analog writes it is an uint8_t. For reads the value field is the one that is being returned.
 */
typedef struct {
	uint8_t cmd;
	uint8_t pin;
	uint8_t opcode;
	uint8_t value;
} pin_cmd_t;

enum CommandMicroappPinOpcde {
	CS_MICROAPP_COMMAND_READ         = 0x01,
	CS_MICROAPP_COMMAND_WRITE        = 0x02,
	CS_MICROAPP_COMMAND_TOGGLE       = 0x03,
};

typedef struct {
	uint8_t cmd;
	uint16_t period;
	uintptr_t coargs;
} sleep_cmd_t;

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
