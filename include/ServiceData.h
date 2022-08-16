#pragma once

#include <Arduino.h>

class ServiceData_ {

private:
	ServiceData_(){};
	ServiceData_(ServiceData_ const&)   = delete;
	void operator=(ServiceData_ const&) = delete;

	void _write(microapp_sdk_service_data_t* serviceData);

public:
	static ServiceData_& getInstance() {
		// Guaranteed to be destroyed.
		static ServiceData_ instance;

		// Instantiated on first use.
		return instance;
	}

	void write(uint16_t appUuid, const char* str);
	void write(uint16_t appUuid, String str);
	void write(uint16_t appUuid, uint8_t* buf, size_t size);
};

#define ServiceData ServiceData_::getInstance()