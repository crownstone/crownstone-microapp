#pragma once

// Get defaults from bluenet
#include <cs_MicroappStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

// Interrupt functions
typedef int (*interruptFunction)(void *, void *);

// Store interrupts in the microapp
struct interrupt_registration_t {
    uint8_t major;
    uint8_t minor;
    interruptFunction interruptFunc;
    void *arg;
    bool registered;
};

#define MAX_INTERRUPT_REGISTRATIONS 4

extern interrupt_registration_t interruptRegistrations[MAX_INTERRUPT_REGISTRATIONS];

// Create shortened typedefs (it is obvious we are within the microapp here)

typedef microapp_sdk_ble_t ble_sdk_t;
typedef microapp_sdk_twi_t twi_sdk_t;
typedef microapp_sdk_pin_t pin_sdk_t;

#define OUTPUT          CS_MICROAPP_SDK_PIN_OUTPUT
#define INPUT           CS_MICROAPP_SDK_PIN_INPUT
#define INPUT_PULLUP    CS_MICROAPP_SDK_PIN_INPUT_PULLUP

#define CHANGE          CS_MICROAPP_SDK_PIN_CHANGE
#define RISING          CS_MICROAPP_SDK_PIN_RISING
#define FALLING         CS_MICROAPP_SDK_PIN_FALLING

#define I2C_INIT        CS_MICROAPP_SDK_TWI_INIT
#define I2C_READ        CS_MICROAPP_SDK_TWI_READ
#define I2C_WRITE       CS_MICROAPP_SDK_TWI_WRITE

const uint8_t LOW = 0;
const uint8_t HIGH = !LOW;

// 10 GPIO pins, 4 buttons, and 4 leds
#define NUMBER_OF_PINS 18

// define size_t as a 16-bit unsigned int
typedef uint16_t size_t;

// redefine the max size of a string
const size_t MAX_STRING_SIZE = MICROAPP_SDK_MAX_STRING_LENGTH;

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
 * @return -1        if for the first unmatching byte i we have ptr1[i] <
 * ptr2[i]
 * @return 1         if for the first unmatching byte i we have ptr1[i] >
 * ptr2[i]
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
void *memcpy(void *dest, const void *src, size_t num);

/*
 * Get outgoing message buffer (can be used for sendMessage);
 */
//io_buffer_t *getOutgoingMessageBuffer();

//io_buffer_t *getIncomingMessageBuffer();

uint8_t *getOutgoingMessagePayload();

uint8_t *getIncomingMessagePayload();

/**
 * Send a message to the bluenet code. This is the function that is called - in
 * the end - by all the functions that have to reach the microapp code.
 */
int sendMessage();

/**
 * Register a softInterrupt locally.
 */
int registerSoftInterrupt(soft_interrupt_t *interrupt);

/**
 * Remove a registered softInterrupt locally
 */
int removeRegisteredSoftInterrupt(uint8_t type, uint8_t id);

/**
 * Handle softInterrupts.
 */
int handleSoftInterrupt(microapp_sdk_header_t *header);

#ifdef __cplusplus
}
#endif
