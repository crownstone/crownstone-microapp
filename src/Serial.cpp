#include <Serial.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

// Design choice is that strings will always be null-terminated.
// The last byte will be overwritten at the bluenet side by a null byte even if this is not done in the microapp code.

#define SERIAL_SIZE_OPCODE                           3
#define SERIAL_MAX_PAYLOAD_LENGTH                    (MAX_PAYLOAD - SERIAL_SIZE_OPCODE)
#define SERIAL_MAX_STRING_LENGTH                     (SERIAL_MAX_PAYLOAD_LENGTH - 1)

int SerialBase_::write(char value) {
	const char buf[1] = { value };
	return _write(reinterpret_cast<const uint8_t*>(buf), 1, Type::Char);
}

int SerialBase_::write(float value) {
	const int32_t casted = (int32_t)(value * 10000); // we just cast to 4 decimals
	return _write(reinterpret_cast<const uint8_t*>(&casted), sizeof(value), Type::Float);
}

int SerialBase_::write(double value) {
	return _write(reinterpret_cast<const uint8_t*>(&value), sizeof(value), Type::Double);
}

int SerialBase_::write(int value) {
	return _write(reinterpret_cast<const uint8_t*>(&value), sizeof(value), Type::Int);
}

int SerialBase_::write(short value) {
	return _write(reinterpret_cast<const uint8_t*>(&value), sizeof(value), Type::Short);
}

int SerialBase_::write(unsigned int value) {
	return _write(reinterpret_cast<const uint8_t*>(&value), sizeof(value), Type::UnsignedInt);
}

int SerialBase_::write(const char *str) {
	return _write(reinterpret_cast<const uint8_t*>(str), strlen(str), Type::Str);
}

int SerialBase_::write(String str, int length) {
	return _write(reinterpret_cast<const uint8_t*>(str.c_str()), length, Type::Str);
}

int SerialBase_::write(const uint8_t *buf, int length) {
	return _write(buf, length, Type::Arr);
}

/// Just copies of above

int SerialBase_::print(String str, int length) {
	return write(str, length);
}

int SerialBase_::print(const char *str) {
	return write(str);
}

int SerialBase_::print(const uint8_t *buf, int length) {
	return write(buf, length);
}

int SerialBase_::print(char value) {
	return write(value);
}

int SerialBase_::print(float value) {
	return write(value);
}

int SerialBase_::print(short value) {
	return write(value);
}

/// In principle, copies of above, but with newlines

int SerialBase_::println(String str, int length) {
	return _write(reinterpret_cast<const uint8_t*>(str.c_str()), length, Type::Str, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(const char *str) {
	return _write(reinterpret_cast<const uint8_t*>(str), strlen(str), Type::Str, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(const uint8_t *buf, int length) {
	return _write(buf, length, Type::Arr, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

/*
int SerialBase_::println(char value) {
	const char buf[1] = { value };
	return _write(reinterpret_cast<const uint8_t*>(buf), 1, Type::Char, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}*/

int SerialBase_::println(uint32_t value) {
	return _write(reinterpret_cast<const uint8_t*>(&value), sizeof(value), Type::UnsignedInt, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int SerialBase_::_write(const uint8_t *buf, int length, Type type, CommandMicroappLogOption option) {
	if (length == 0) {
		// Nothing to send.
		return 0;
	}

	// Set the header.
	global_msg.payload[0] = _port;
	global_msg.payload[1] = type;
	global_msg.payload[2] = option;

	// Make sure length is not too large. Do not silently fail.
	if (type == Type::Str && length > SERIAL_MAX_STRING_LENGTH) {
		length = SERIAL_MAX_STRING_LENGTH;
	}

	// Make sure that in all cases that length is truncated. Do not silently fail.
	if (length > SERIAL_MAX_PAYLOAD_LENGTH) {
		length = SERIAL_MAX_PAYLOAD_LENGTH;
	}
	
	// Copy the data.
	for (int i = 0; i < length; ++i) {
		global_msg.payload[i + SERIAL_SIZE_OPCODE] = buf[i];
	}
	
	global_msg.length = length + SERIAL_SIZE_OPCODE;

	// TODO: check result.
	sendMessage(&global_msg);
	return length;
}
