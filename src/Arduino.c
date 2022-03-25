#include <Arduino.h>
#include <microapp.h>

#include <ipc/cs_IpcRamData.h>

bool pinExists(uint8_t pin) {
	// very basic, should be roundtrip as well
	return (pin < NUMBER_OF_PINS);
}

void pinMode(uint8_t pin, uint8_t mode) {
	if (!pinExists(pin)) return;
	
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_MODE;
	pin_cmd->opcode2 = mode;
	pin_cmd->value = 0;

	sendMessage();
}

void digitalWrite(uint8_t pin, uint8_t val) {
	if (!pinExists(pin)) return;

	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_ACTION;
	pin_cmd->opcode2 = CS_MICROAPP_COMMAND_PIN_WRITE;
	pin_cmd->value = val;

	sendMessage();
}

int digitalRead(uint8_t pin) {
	if (!pinExists(pin)) return -1;
	
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
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

int attachInterrupt(uint8_t pin, void (*isr)(void), uint8_t mode) {
	if (!pinExists(pin)) return -1;

	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_pin_cmd_t* pin_cmd = reinterpret_cast<microapp_pin_cmd_t*>(payload);
	pin_cmd->header.cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode1 = CS_MICROAPP_COMMAND_PIN_MODE;
	pin_cmd->opcode2 = CS_MICROAPP_COMMAND_PIN_INPUT_PULLUP;
	pin_cmd->value = mode;
	
	soft_interrupt_t interrupt;
	interrupt.type = SOFT_INTERRUPT_TYPE_PIN;
	interrupt.id = pin_cmd->pin;
	interrupt.softInterruptFunc = reinterpret_cast<softInterruptFunction>(isr);
	registerSoftInterrupt(&interrupt);

	int result = sendMessage();
	return result;
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

uint8_t digitalPinToInterrupt(uint8_t pin) {
	return pin + 1;
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

