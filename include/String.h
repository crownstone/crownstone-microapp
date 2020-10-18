#pragma once

class String {

	const char *_str;

	public:
		String(const char* str) {
			_str = str;
		}

		const char* c_str() {
			return _str;
		}
};
