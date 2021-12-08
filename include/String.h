#pragma once

#include <microapp.h>

class String {

	const char *_str; // actual buffer
	unsigned int  _len; // length of string

	public:
		String(const char* str) {
			_len = strlen(str);
			_str = str;
		}

		const char* c_str() {
			return _str;
		}

		unsigned int length() {
			return _len;
		}
};
