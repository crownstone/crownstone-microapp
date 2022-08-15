#pragma once

#include <String.h>
#include <stdint.h>
#include <cs_MicroappStructs.h>

class Serial_ {
private:
	Serial_(){};
	Serial_(Serial_ const&)         = delete;
	void operator=(Serial_ const&)  = delete;

	enum Type {
		Char        = CS_MICROAPP_SDK_LOG_CHAR,
		Int         = CS_MICROAPP_SDK_LOG_INT,
		Str         = CS_MICROAPP_SDK_LOG_STR,
		Arr         = CS_MICROAPP_SDK_LOG_ARR,
		Float       = CS_MICROAPP_SDK_LOG_FLOAT,
		Double      = CS_MICROAPP_SDK_LOG_DOUBLE,
		UnsignedInt = CS_MICROAPP_SDK_LOG_UINT,
		Short       = CS_MICROAPP_SDK_LOG_SHORT,
	};

	size_t _write(microapp_sdk_log_header_t *logRequest, Type type, MicroappSdkLogFlags flags);
	size_t _write(String str, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(const char *str, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(const uint8_t *buf, int length, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(char value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(float value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(double value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(short value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(int value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);
	size_t _write(unsigned int value, MicroappSdkLogFlags flags = CS_MICROAPP_SDK_LOG_FLAG_CLEAR);

public:
	static Serial_ & getInstance() {
		// Guaranteed to be destroyed.
		static Serial_ instance;

		// Instantiated on first use.
		return instance;
	}

	// TODO: make a roundtrip to bluenet to check if the crownstone has a port to log over
	void begin();

	// Returns always true for now.
	// Can be used as
	//   if(Serial)
	// Might return false in release mode (when there is no logging available)
	// However, in that case we might still want to get data out over Bluetooth
	// TODO: return the result from begin()
	explicit operator bool() const { return true; }

	// Write a single byte to serial.
	// Returns number of bytes written.
	size_t write(char value);

	size_t write(float value);

	size_t write(double value);

	size_t write(short value);

	size_t write(int value);

	size_t write(unsigned int value);

	// Write a string (as char array) to serial. The length will be obtained through searching for a null
	// byte.
	// Returns number of bytes written.
	size_t write(const char *str);

	// Write to serial. For now this becomes logs in the Crownstone firmware. That is not so useful to the
	// microapp person though. To send it through to UART for a USB dongle is quite limited, for normal
	// Crownstones it is almost useless. It would be fun to write over Bluetooth RFCOMM.
	//
	// Returns number of bytes written.
	size_t write(String str);

	// Write an array of bytes to serial.
	// Returns number of bytes written.
	size_t write(const uint8_t *buf, int length);

	// Copies of write

	size_t print(char value);

	size_t print(float value);

	size_t print(double value);

	size_t print(short value);

	size_t print(int value);

	size_t print(unsigned int value);

	size_t print(const char *str);

	size_t print(String str);

	size_t print(const uint8_t *buf, int length);

	// Copies of print (but with newline)

	size_t println(char value);

	size_t println(float value);

	size_t println(double value);

	size_t println(short value);

	size_t println(int value);

	size_t println(unsigned int value);

	size_t println(const char *str);

	size_t println(String str);

	size_t println(const uint8_t *buf, int length);


};

#define Serial Serial_::getInstance()

