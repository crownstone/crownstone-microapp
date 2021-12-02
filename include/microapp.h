#pragma once

// Get defaults from bluenet
#include <cs_MicroappStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OUTPUT               CS_MICROAPP_COMMAND_PIN_WRITE
#define INPUT                CS_MICROAPP_COMMAND_PIN_READ
#define TOGGLE               CS_MICROAPP_COMMAND_PIN_TOGGLE
#define INPUT_PULLUP         CS_MICROAPP_COMMAND_PIN_INPUT_PULLUP

#define CHANGE               CS_MICROAPP_COMMAND_VALUE_CHANGE
#define RISING               CS_MICROAPP_COMMAND_VALUE_RISING
#define FALLING              CS_MICROAPP_COMMAND_VALUE_FALLING

#define I2C_INIT             CS_MICROAPP_COMMAND_TWI_INIT
#define I2C_READ             CS_MICROAPP_COMMAND_TWI_READ
#define I2C_WRITE            CS_MICROAPP_COMMAND_TWI_WRITE

//#define HIGH                 CS_MICROAPP_COMMAND_SWITCH_ON
//#define LOW                  CS_MICROAPP_COMMAND_SWITCH_OFF

const uint8_t LOW = 0;
const uint8_t HIGH = !LOW;

// 1 virtual pin, 4 GPIO pins, 4 buttons, and 4 leds
#define NUMBER_OF_PINS 13

// define size_t as a 16-bit unsigned int
typedef uint16_t size_t;

// returns size MAX_PAYLOAD for strings that are too long
uint8_t strlen(const char *str);

// compares the first len bytes of bufA and bufB and returns zero if they match
int memcmp(const void *bufA, const void *bufB, size_t len);

// copies the string at src to dest
char* strcpy (char* dest, const char* src);

/*
 * To save space have a single global message object.
 */
extern microapp_message_t global_msg;

/*
 * Send a message to the bluenet code. This is the function that is called - in the end - by all the functions
 * that have to reach the microapp code.
 */
int sendMessage(microapp_message_t *msg);

#ifdef __cplusplus
}
#endif
