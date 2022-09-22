#include <Arduino.h>
#include <ipc/cs_IpcRamData.h>
#include <microapp.h>

/*
 * Implementation sends a message under the hood. The microapp itself is reponsible for looping for long enough.
 * A tick takes 100 ms. Hence the loop should be the delay in ms divided by 100.
 */
void delay(uint32_t delay_ms) {
	const uint8_t bluenet_tick_duration_ms = 100;
	uint32_t ticks                         = delay_ms / (bluenet_tick_duration_ms * MICROAPP_LOOP_FREQUENCY);
	for (uint32_t i = 0; i < ticks; i++) {
		uint8_t* payload            = getOutgoingMessagePayload();
		microapp_sdk_yield_t* yield = (microapp_sdk_yield_t*)(payload);
		yield->header.messageType   = CS_MICROAPP_SDK_TYPE_YIELD;
		yield->type                 = CS_MICROAPP_SDK_YIELD_ASYNC;
		yield->emptyInterruptSlots  = emptySlotsInStack();
		sendMessage();
	}
}

bool pinExists(uint8_t pin) {
	// First check, more checks on bluenet side
	return (pin < NUMBER_OF_PINS);
}

// Convert a pin to a virtual pin ('interrupt' in Arduino language)
// This is a trivial mapping, here to comply with Arduino syntax
uint8_t digitalPinToInterrupt(uint8_t pin) {
	return pin;
}

// Convert a virtual pin back to a pin
// This is a trivial mapping, see digitalPinToInterrupt()
uint8_t interruptToDigitalPin(uint8_t interrupt) {
	return interrupt;
}

// The mode here is INPUT, OUTPUT, INPUT_PULLUP, etc.
void pinMode(uint8_t pin, uint8_t mode) {
	if (!pinExists(pin)) {
		return;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_pin_t* pinRequest = reinterpret_cast<microapp_sdk_pin_t*>(payload);
	pinRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	pinRequest->header.messageType = CS_MICROAPP_SDK_TYPE_PIN;
	pinRequest->pin                = pin;
	pinRequest->type               = CS_MICROAPP_SDK_PIN_INIT;
	pinRequest->direction          = mode;
	pinRequest->polarity           = CS_MICROAPP_SDK_PIN_NO_POLARITY;

	sendMessage();
}

void digitalWrite(uint8_t pin, uint8_t val) {
	if (!pinExists(pin)) {
		return;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_pin_t* pinRequest = reinterpret_cast<microapp_sdk_pin_t*>(payload);
	pinRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	pinRequest->header.messageType = CS_MICROAPP_SDK_TYPE_PIN;
	pinRequest->pin                = pin;
	pinRequest->type               = CS_MICROAPP_SDK_PIN_ACTION;
	pinRequest->action             = CS_MICROAPP_SDK_PIN_WRITE;
	if (val == HIGH) {
		pinRequest->value = CS_MICROAPP_SDK_PIN_ON;
	}
	else {
		pinRequest->value = CS_MICROAPP_SDK_PIN_OFF;
	}
	sendMessage();
}

int digitalRead(uint8_t pin) {
	if (!pinExists(pin)) {
		return -1;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_pin_t* pinRequest = reinterpret_cast<microapp_sdk_pin_t*>(payload);
	pinRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	pinRequest->header.messageType = CS_MICROAPP_SDK_TYPE_PIN;
	pinRequest->pin                = pin;
	pinRequest->type               = CS_MICROAPP_SDK_PIN_ACTION;
	pinRequest->action             = CS_MICROAPP_SDK_PIN_READ;
	pinRequest->value              = 0;

	sendMessage();

	if (pinRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return -1;
	}
	// TODO, perhaps a larger type then uint8_t is required / desired
	uint8_t value = pinRequest->value;
	return value;
}

/**
 * The mode here is LOW, CHANGE, RISING, FALLING, HIGH.
 *
 * Actually, this again sets also the values that are set with pinMode. That's redundant.
 * For now, just keep it like this because it doesn't hurt to have a pin configured twice.
 */
bool attachInterrupt(uint8_t interruptIndex, void (*isr)(void), uint8_t mode) {
	if (!pinExists(interruptToDigitalPin(interruptIndex))) {
		return false;
	}

	interrupt_registration_t interrupt;
	interrupt.type          = CS_MICROAPP_SDK_TYPE_PIN;
	interrupt.id          = interruptIndex;
	interrupt.handler        = reinterpret_cast<interruptFunction>(isr);
	microapp_sdk_result_t result = registerInterrupt(&interrupt);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_pin_t* pinRequest = reinterpret_cast<microapp_sdk_pin_t*>(payload);
	pinRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	pinRequest->header.messageType = CS_MICROAPP_SDK_TYPE_PIN;
	pinRequest->pin                = interruptIndex;
	pinRequest->type               = CS_MICROAPP_SDK_PIN_INIT;
	pinRequest->direction          = CS_MICROAPP_SDK_PIN_INPUT_PULLUP;
	pinRequest->polarity           = mode;

	result = sendMessage();
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// Remove locally registered interrupt
		result = removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_PIN, interruptIndex);
		// Do nothing with the result. We return false anyway
		return false;
	}
	return true;
}

// Internally it is just the same as digitalRead
int analogRead(uint8_t pin) {
	return digitalRead(pin);
}

void analogReference(uint8_t mode) {}

// Internally it is just the same as digitalWrite
void analogWrite(uint8_t pin, int val) {
	digitalWrite(pin, val);
}

void init() {}

void initVariant() {}

// Return highest byte (8 bits) or second-lowest byte (for larger data types)
byte highByte(short val) {
	return (val >> 8);
}

// Return lowest byte (8 bits)
byte lowByte(short val) {
	return (val && 0xFF);
}

short word(byte highByte, byte lowByte) {
	return (short)(highByte << 8) | lowByte;
}
