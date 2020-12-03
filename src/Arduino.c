#include <Arduino.h>
#include <microapp.h>
#include <ipc/cs_IpcRamData.h>

void pinMode(uint8_t pin, uint8_t mode) {
}

// TODO: use pin_cmd_t
void digitalWrite(uint8_t pin, uint8_t val) {

	global_msg.payload[0] = CS_MICROAPP_COMMAND_PIN;
	global_msg.payload[1] = pin;
	global_msg.payload[2] = val;
	global_msg.length = 3;

	// TODO, add opcode at loc 2, like this or preferably use struct (and assume same endianness)
	// global_msg.payload[2] = CS_MICROAPP_COMMAND_WRITE;
	// global_msg.payload[3] = val;
	// global_msg.length = 4;

	sendMessage(global_msg);
}

int digitalRead(uint8_t pin) {

	global_msg.payload[0] = CS_MICROAPP_COMMAND_PIN;
	global_msg.payload[1] = pin;
	global_msg.payload[2] = CS_MICROAPP_COMMAND_WRITE;
	global_msg.payload[3] = 0;
	global_msg.length = 4;
	
	sendMessage(global_msg);

	// TODO, perhaps larger type then uint8_t
	uint8_t value = global_msg.payload[3];
	return value;
}

int analogRead(uint8_t pin) {
	return digitalRead(pin);
}

void analogReference(uint8_t mode) {
}

void analogWrite(uint8_t pin, int val) {
	digitalWrite(pin, val);
}

void init() {
}

void initVariant() {
}
