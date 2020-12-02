#include <Serial.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

#define SIZE_OPCODE                           2
#define MAX_SERIAL_PAYLOAD_LENGTH             (MAX_PAYLOAD - SIZE_OPCODE)
#define MAX_STRING_LENGTH                     (MAX_SERIAL_PAYLOAD_LENGTH - 1)

// returns size 0 for strings that are too long
// TODO: this doesn't allow us to truncate strings though, maybe return MAX_STRING_LENGTH instead?
uint8_t strlen(const char *str) {
	for (uint8_t i = 0; i < MAX_STRING_LENGTH + 1; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return 0;
}

void SerialBase_::write(char value) {
	const char buf[1] = { value };
	_write(buf, 1, Type::Char);
}

int SerialBase_::write(const char *str) {
	return _write(str, strlen(str) + 1, Type::Str);
}

int SerialBase_::write(String str, int length) {
	return _write(str.c_str(), length + 1, Type::Str);
}

int SerialBase_::write(const char *str, int length) {
	return _write(str, length, Type::Arr);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
// TODO: shouldn't this return the number of bytes written as size_t?
//
int SerialBase_::_write(const char *str, int length, Type type) {
	// This check is required in order to use "length - 1".
	if (length == 0) {
		// Nothing to send.
		return 0;
	}

	global_msg.payload[0] = _port;
	global_msg.payload[1] = type;

	// Make sure length is not too large.
	if (length > MAX_SERIAL_PAYLOAD_LENGTH) {
		return 0;
		length = MAX_SERIAL_PAYLOAD_LENGTH;
	}
	
	// Copy the data.
	for (int i = 0; i < length; ++i) {
		global_msg.payload[i + SIZE_OPCODE] = str[i];
	}
	
	// Make sure strings are null terminated.
	if (type == Type::Str) {
		global_msg.payload[length - 1] = 0;
	}
	
	global_msg.length = length + SIZE_OPCODE;

	return sendMessage(global_msg);
}
