#pragma once

// Get defaults from bluenet
#include <cs_MicroappStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct message_t {
	uint8_t payload[MAX_PAYLOAD];
	uint16_t length;
};

// Callback functions
typedef void (*callbackFunction)(void*);

const uint8_t CALLBACK_TYPE_BLE = 1;
const uint8_t CALLBACK_TYPE_PIN = 2;

// Store callbacks in the microapp
struct callback_t {
	uint8_t type;
	uint8_t id;
	callbackFunction callback;
	void *arg;
	bool registered;
};

#define MAX_CALLBACKS 4

extern callback_t callbacks[MAX_CALLBACKS];

// Create shortened typedefs (it is obvious we are within the microapp here)

typedef microapp_ble_cmd_t ble_cmd_t;
typedef microapp_twi_cmd_t twi_cmd_t;
typedef microapp_pin_cmd_t pin_cmd_t;
typedef microapp_ble_device_t ble_dev_t;

// Create long-form version for who wants

typedef message_t microapp_message_t;

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

// set a max string size which is equal to the max payload of microapp_message_t
const size_t MAX_STRING_SIZE = MAX_PAYLOAD;

/**
 * Finds the length of a null-terminated string
 *
 * @param[in] str    const char pointer to the first character
 *
 * @return           length of the string
 */
uint8_t strlen(const char *str);

/**
 * Compares the first num bytes of ptr1 and ptr2
 *
 * @param[in] ptr1   Pointer to block of memory
 * @param[in] ptr2   Pointer to block of memory
 * @param[in] num    The number of bytes to compare
 *
 * @return 0         if ptr1 and ptr2 are equal
 * @return -1        if for the first unmatching byte i we have ptr1[i] < ptr2[i]
 * @return 1         if for the first unmatching byte i we have ptr1[i] > ptr2[i]
 */
int memcmp(const void *ptr1, const void *ptr2, size_t num);

/**
 * Copies num bytes from src to dest
 *
 * @param[in] dest   The starting address to copy data to
 * @param[in] src    The starting address from where to copy data
 * @param[in] num    The number of bytes to copy
 *
 * @return           A pointer to dest
 */
void* memcpy(void* dest, const void* src, size_t num);

/*
 * To save space have a single global message object.
 */
extern microapp_message_t global_msg;

/**
 * Send a message to the bluenet code. This is the function that is called - in the end - by all the functions
 * that have to reach the microapp code.
 */
int sendMessage(microapp_message_t *msg);

/**
 * Register a callback locally so that when a message.
 */
void registerCallback(callback_t *cb);

/**
 * Evoke a previously registered callback.
 */
int evokeCallback(uint8_t type, uint8_t id);

/**
 * Handle callbacks.
 */
int handleCallbacks(microapp_cmd_t *msg);

#ifdef __cplusplus
}
#endif
