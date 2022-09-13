#pragma once

#include <microapp.h>

class String {

	char* _str;         // actual buffer
	unsigned int _len;  // length of string

public:
	String(const char* str) {
		_len = strlen(str);
		_str = (char*)str;
	}

	String(char* str, unsigned int len) {
		_len      = len;
		_str      = str;
		// this is dangerous, we're writing to a memory address outside
		// of the indicated len!
		_str[len] = 0;
	}

	const char* c_str() { return _str; }

	unsigned int length() { return _len; }
};
