#include <Serial.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

#define SIZE_OPCODE                           2
#define MAX_SERIAL_PAYLOAD_LENGTH             (MAX_PAYLOAD - SIZE_OPCODE)
#define MAX_STRING_LENGTH                     (MAX_SERIAL_PAYLOAD_LENGTH - 1) // Strings need space for a null termination.

// returns size MAX_STRING_LENGTH for strings that are too long
uint8_t strlen(const char *str) {
	for (uint8_t i = 0; i < MAX_STRING_LENGTH + 1; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return MAX_STRING_LENGTH;
}

int SerialBase_::write(char value) {
	const char buf[1] = { value };
	return _write(reinterpret_cast<const uint8_t*>(buf), 1, Type::Char);
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

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int SerialBase_::_write(const uint8_t *buf, int length, Type type) {
	if (length == 0) {
		// Nothing to send.
		return 0;
	}

	// Set the header.
	global_msg.payload[0] = _port;
	global_msg.payload[1] = type;

	// Make sure length is not too large.
	if (type == Type::Str && length > MAX_STRING_LENGTH) {
		// Strings can be truncated.
		length = MAX_STRING_LENGTH;
	}
	else if (length > MAX_SERIAL_PAYLOAD_LENGTH) {
		return 0;
	}
	
	// Copy the data.
	for (int i = 0; i < length; ++i) {
		global_msg.payload[i + SIZE_OPCODE] = buf[i];
	}
	
	global_msg.length = length + SIZE_OPCODE;

	// TODO: check result.
	sendMessage(global_msg);
	return length;
}
