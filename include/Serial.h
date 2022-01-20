#pragma once

#include <String.h>
#include <stdint.h>
#include <cs_MicroappStructs.h>

class SerialBase_ {
public:
	void begin();

	// Returns always true for now.
	// Can be used as
	//   if(Serial)
	// Might return false in release mode (when there is no logging available)
	// However, in that case we might still want to get data out over Bluetooth
	explicit operator bool() const { return true; }

	// Write a single byte to serial.
	// Returns number of bytes written.
	int write(char value);

	int write(float value);

	int write(double value);

	int write(short value);

	int write(int value);

	int write(unsigned int value);
	
	// Write a string (as char array) to serial. The length will be obtained through searching for a null
	// byte.
	// Returns number of bytes written.
	int write(const char *str);

	// Write to serial. For now this becomes logs in the Crownstone firmware. That is not so useful to the
	// microapp person though. To send it through to UART for a USB dongle is quite limited, for normal 
	// Crownstones it is almost useless. It would be fun to write over Bluetooth RFCOMM.
	//
	// Returns number of bytes written.
	int write(String str, int length);

	// Write an array of bytes to serial.
	// Returns number of bytes written.
	int write(const uint8_t *buf, int length);

	// Copies of write

	int print(char value);

	int print(float value);

	int print(double value);

	int print(short value);

	int print(int value);

	int print(unsigned int value);

	int print(const char *str);

	int print(String str, int length);

	int print(const uint8_t *buf, int length);

	// Copies of print (but with newline)

	int println(char value);

	int println(float value);

	int println(double value);

	int println(short value);

	int println(int value);

	int println(unsigned int value);

	int println(const char *str);

	int println(String str, int length);

	int println(const uint8_t *buf, int length);

private:
	// Defines which serial this is. Must be equal to command number.
	char _port;

protected:
	SerialBase_(char port) : _port(port) {}

	enum Type { Char = 0, Int = 1, Str = 2, Arr = 3, Float = 4, Double = 5, UnsignedInt = 6, Short = 7};

	int _write(microapp_log_cmd_t *cmd, Type type, CommandMicroappLogOption option);

	int _write(String str, int length, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(const char *str, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(const uint8_t *buf, int length, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(char value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(float value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(double value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(short value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(int value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);

	int _write(unsigned int value, CommandMicroappLogOption option = CS_MICROAPP_COMMAND_LOG_NO_NEWLINE);
};

/**
 * Uses a SerialBase_ instance internally with port number 1.
 */
class Serial_: public SerialBase_ {
public:
	static Serial_& getInstance()
	{
		// Guaranteed to be destroyed.
		static Serial_ instance(MICROAPP_SERIAL_DEFAULT_PORT_NUMBER);

		// Instantiated on first use.
		return instance;
	}

private:
	Serial_(char port) : SerialBase_(port) {}
	Serial_(Serial_ const&)         = delete;
	void operator=(Serial_ const&)  = delete;
};

/**
 * Uses a SerialBase_ instance internally with port number 4.
 */
class SerialServiceData_: public SerialBase_ {
public:
	static SerialServiceData_& getInstance()
	{
		// Guaranteed to be destroyed.
		static SerialServiceData_ instance(MICROAPP_SERIAL_SERVICE_DATA_PORT_NUMBER);

		// Instantiated on first use.
		return instance;
	}

	int write(microapp_service_data_t *data);

private:
	SerialServiceData_(char port) : SerialBase_(port) {}
	SerialServiceData_(SerialServiceData_ const&) = delete;
	void operator=(SerialServiceData_ const&)     = delete;
};

#define Serial Serial_::getInstance()
#define SerialServiceData SerialServiceData_::getInstance()

