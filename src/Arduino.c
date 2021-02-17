#include <Arduino.h>
#include <microapp.h>

#include <ipc/cs_IpcRamData.h>

static uint8_t pin_modes[NUMBER_OF_PINS];

bool pinExists(uint8_t pin) {
	return (pin < NUMBER_OF_PINS);
}

// TODO: check also correctness of "mode"
void pinMode(uint8_t pin, uint8_t mode) {
	if (!pinExists(pin)) return;
	pin_modes[pin] = mode;
}

void digitalWrite(uint8_t pin, uint8_t val) {
	if (!pinExists(pin)) return;

	pin_cmd_t *pin_cmd = (pin_cmd_t*)&global_msg;
	pin_cmd->cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode = pin_modes[pin];
	pin_cmd->value = val;
	global_msg.length = sizeof(pin_cmd_t);

	sendMessage(&global_msg);
}

int digitalRead(uint8_t pin) {
	if (!pinExists(pin)) return -1;
	
	pin_cmd_t *pin_cmd = (pin_cmd_t*)&global_msg;
	pin_cmd->cmd = CS_MICROAPP_COMMAND_PIN;
	pin_cmd->pin = pin;
	pin_cmd->opcode = pin_modes[pin];
	pin_cmd->value = 0;
	global_msg.length = sizeof(pin_cmd_t);
	
	sendMessage(&global_msg);

	// TODO, perhaps a larger type then uint8_t is required / desired
	uint8_t value = pin_cmd->value;
	return value;
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

