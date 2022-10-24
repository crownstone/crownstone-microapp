#pragma once

// Get defaults from bluenet
#include <cs_MicroappStructs.h>

#ifdef __cplusplus
extern "C" {
#endif

// Interrupt functions
typedef microapp_sdk_result_t (*interruptFunction)(void*);

// Store interrupts in the microapp
struct interrupt_registration_t {
	MicroappSdkType type;
	uint8_t id;
	interruptFunction handler;
	bool registered;
};

#define MAX_INTERRUPT_REGISTRATIONS 6

extern interrupt_registration_t interruptRegistrations[MAX_INTERRUPT_REGISTRATIONS];

// define microapp_size_t as a 16-bit unsigned int
typedef uint16_t microapp_size_t;

// redefine the max size of a string
const microapp_size_t MAX_STRING_SIZE = MICROAPP_SDK_MAX_STRING_LENGTH;

/**
 * Finds the length of a null-terminated string
 *
 * @param[in] str    const char pointer to the first character
 *
 * @return           length of the string
 */
uint8_t strlen(const char* str);

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
int memcmp(const void* ptr1, const void* ptr2, microapp_size_t num);

/**
 * Copies num bytes from src to dest
 *
 * @param[in] dest   The starting address to copy data to
 * @param[in] src    The starting address from where to copy data
 * @param[in] num    The number of bytes to copy
 *
 * @return           A pointer to dest
 */
void* memcpy(void* dest, const void* src, microapp_size_t num);

/*
 * Get outgoing message buffer (can be used for sendMessage);
 */

uint8_t* getOutgoingMessagePayload();

uint8_t* getIncomingMessagePayload();

/**
 * Send a message to the bluenet code. This is the function that is called - in
 * the end - by all the functions that have to reach the microapp code.
 */
microapp_sdk_result_t sendMessage();

/*
 * Returns the number of empty slots for bluenet.
 */
uint8_t emptySlotsInStack();

/**
 * Register a softInterrupt locally.
 */
microapp_sdk_result_t registerInterrupt(interrupt_registration_t* interrupt);

/**
 * Remove a registered softInterrupt locally
 */
microapp_sdk_result_t removeInterruptRegistration(MicroappSdkType type, uint8_t id);

/**
 * Handle interrupts
 */
microapp_sdk_result_t handleInterrupt(microapp_sdk_header_t* header);

#ifdef __cplusplus
}
#endif
