#include <Serial.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

#define SIZE_OPCODE                           2
#define MAX_STRING_LENGTH                     (MAX_PAYLOAD - SIZE_OPCODE)

// returns size 0 for strings that are too long
uint8_t strlen(const char *str) {
	for (uint8_t i = 0; i < MAX_STRING_LENGTH; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return 0;
}

void Serial_::write(char value) {
	const char buf[1] = { value };
	_write(buf, 1, Char);
}

int Serial_::write(const char *str) {
	return _write(str, strlen(str), Type::Str);
}

int Serial_::write(String str, int length) {
	return _write(str.c_str(), length, Type::Str);
}

int Serial_::write(const char *str, int length) {
	return _write(str, length, Type::Str);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int Serial_::_write(const char *str, int length, Type type) {
	global_msg.payload[0] = 1;
	global_msg.payload[1] = type;

	for (int i = 0; i < length; ++i) {
		global_msg.payload[i + SIZE_OPCODE] = str[i];
	}
	global_msg.length = length + SIZE_OPCODE;
	if (global_msg.length <= MAX_PAYLOAD) {
		global_msg.payload[global_msg.length] = 0;
	} else {
		global_msg.length = MAX_PAYLOAD;
	}
	return sendMessage(global_msg);
}
