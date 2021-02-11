#include <Wire.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

// Design choice is that strings will always be null-terminated.
// The last byte will be overwritten at the bluenet side by a null byte even if this is not done in the microapp code.

#define WIRE_SIZE_OPCODE                           6
#define WIRE_MAX_PAYLOAD_LENGTH                    (MAX_PAYLOAD - WIRE_SIZE_OPCODE)
#define WIRE_MAX_STRING_LENGTH                     (WIRE_MAX_PAYLOAD_LENGTH - 1)

int WireBase_::write(char value) {
	const char buf[1] = { value };
	return _write(reinterpret_cast<const uint8_t*>(buf), 1, Type::Char);
}

int WireBase_::write(const char *str) {
	return _write(reinterpret_cast<const uint8_t*>(str), strlen(str), Type::Str);
}

int WireBase_::write(String str, int length) {
	return _write(reinterpret_cast<const uint8_t*>(str.c_str()), length, Type::Str);
}

int WireBase_::write(const uint8_t *buf, int length) {
	return _write(buf, length, Type::Arr);
}

int WireBase_::send(char value) {
	return write(value);
}

int WireBase_::send(const char *str) {
	return write(str);
}

int WireBase_::send(String str, int length) {
	return write(str, length);
}

int WireBase_::send(const uint8_t *buf, int length) {
	return write(buf, length);
}

void WireBase_::begin() {
	twi_cmd_t *twi_cmd = (twi_cmd_t*)&global_msg;
	twi_cmd->cmd = CS_MICROAPP_COMMAND_TWI;
	twi_cmd->address = 0;
	twi_cmd->opcode = I2C_INIT;
	twi_cmd->ack = 0;
	twi_cmd->stop = true;
	sendMessage(global_msg);
}

//void WireBase_::begin(uint8_t address) {
//}

void WireBase_::beginTransmission(const uint8_t address) {
	_address = address;
}

void WireBase_::endTransmission() {
	// dummy for now
}

void WireBase_::requestFrom(const uint8_t address, const int size, bool stop) {
}

int WireBase_::available() {
	return 0;
}

const uint8_t WireBase_::read() {
	return 0;
}

//
// Write message.  We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int WireBase_::_write(const uint8_t *buf, int length, Type type) {
	if (length == 0) {
		// Nothing to send.
		return 0;
	}
	
	twi_cmd_t *twi_cmd = (twi_cmd_t*)&global_msg;
	twi_cmd->cmd = CS_MICROAPP_COMMAND_TWI;
	twi_cmd->address = _address;
	twi_cmd->opcode = I2C_WRITE;
	twi_cmd->ack = 0;
	twi_cmd->stop = true;

	// Make sure length is not too large.
	if (type == Type::Str && length > WIRE_MAX_STRING_LENGTH) {
		length = WIRE_MAX_STRING_LENGTH;
	}
	
	// Make sure that in all cases that length is truncated. Do not silently fail.
	if (length > WIRE_MAX_PAYLOAD_LENGTH) {
		length = WIRE_MAX_PAYLOAD_LENGTH;
	}
	
	twi_cmd->length = length;
	
	// Copy the data.
	for (int i = 0; i < length; ++i) {
		global_msg.payload[i + WIRE_SIZE_OPCODE] = buf[i];
	}
	
	global_msg.length = length + WIRE_SIZE_OPCODE;

	// TODO: check result.
	sendMessage(global_msg);
	return length;
}
