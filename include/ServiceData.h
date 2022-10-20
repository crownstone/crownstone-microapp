#pragma once

#include <Arduino.h>

/**
 * Singleton class for advertising service data
 */
class ServiceData_ {

private:
	// Constructor and copy constructor
	ServiceData_(){};
	ServiceData_(ServiceData_ const&)   = delete;
	void operator=(ServiceData_ const&) = delete;

	/**
	 * Write to bluenet to request advertising service data
	 */
	void _write(microapp_sdk_service_data_t* serviceData);

public:
	static ServiceData_& getInstance() {
		// Guaranteed to be destroyed.
		static ServiceData_ instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Public functions for writing service data
	 * Supported are string literals, String objects and byte arrays
	 */
	void write(uint16_t appUuid, const char* str);
	void write(uint16_t appUuid, String str);
	void write(uint16_t appUuid, uint8_t* buf, microapp_size_t size);
};

#define ServiceData ServiceData_::getInstance()