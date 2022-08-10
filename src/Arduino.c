#include <Arduino.h>
#include <microapp.h>

#include <ipc/cs_IpcRamData.h>

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

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_MODE;
	pin_cmd->opcode2 = mode;
	pin_cmd->value = 0;

	sendMessage();
}

void digitalWrite(uint8_t pin, uint8_t val) {
	if (!pinExists(pin)) {
		return;
	}

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_ACTION;
	pin_cmd->opcode2 = CS_MICROAPP_COMMAND_PIN_WRITE;
	pin_cmd->value = val;

	sendMessage();
}

int digitalRead(uint8_t pin) {
	if (!pinExists(pin)) {
		return -1;
	}

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_ACTION;
	pin_cmd->opcode2 = CS_MICROAPP_COMMAND_PIN_READ;
	pin_cmd->value = 0;

	sendMessage();

	// TODO, perhaps a larger type then uint8_t is required / desired
	uint8_t value = pin_cmd->value;
	return value;
}

/**
 * The mode here is LOW, CHANGE, RISING, FALLING, HIGH.
 *
 * Actually, this again sets also the values that are set with pinMode. That's redundant.
 * For now, just keep it like this because it doesn't hurt to have a pin configured twice.
 */
bool attachInterrupt(uint8_t interrupt, void (*isr)(void), uint8_t mode) {
	if (!pinExists(interruptToDigitalPin(interrupt))) {
		return false;
	}

	interrupt_registration_t softInterrupt;
	softInterrupt.type = SOFT_INTERRUPT_TYPE_PIN;
	softInterrupt.id = interrupt;
	softInterrupt.softInterruptFunc = reinterpret_cast<softInterruptFunction>(isr);
	int result = registerSoftInterrupt(&softInterrupt);
	if (result != ERR_MICROAPP_SUCCESS) {
		return false;
	}

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = interrupt;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_MODE;
	pin_cmd->opcode2 = CS_MICROAPP_COMMAND_PIN_INPUT_PULLUP;
	pin_cmd->value = mode;

	result = sendMessage();
	if (result != ERR_MICROAPP_SUCCESS) {
		// Remove locally registered interrupt
		result = removeRegisteredSoftInterrupt(SOFT_INTERRUPT_TYPE_PIN, interrupt);
		// Do nothing with the result. We return false anyway
		return false;
	}
	return true;
}


// Internally it is just the same as digitalRead
int analogRead(uint8_t pin) {
	return digitalRead(pin);
}

void analogReference(uint8_t mode) {
}

// Internally it is just the same as digitalWrite
void analogWrite(uint8_t pin, int val) {
	digitalWrite(pin, val);
}

void init() {
}

void initVariant() {
}

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

