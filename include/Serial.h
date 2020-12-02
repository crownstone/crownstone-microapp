#pragma once

#include <String.h>

class SerialBase_ {
private:
	// Defines which serial this is.
	char _port;
	
protected:
	SerialBase_(char port) : _port(port) {}

	enum Type { Char = 0, Int = 1, Str = 2, Arr = 3};
	
	// Write an array of bytes to serial. 
	int _write(const char *buf, int length, Type type);

	// Do not implement the following functions
	//Serial_(Serial_ const&);
	//void operator=(Serial_ const&);

public:

	// Dummy
	void begin() {};

	// Returns always true for now.
	// Can be used as
	//   if(Serial)
	// Might return false in release mode (when there is no logging available)
	// However, in that case we might still want to get data out over Bluetooth
	explicit operator bool() const { return true; }

	//
	// Write to serial. For now this becomes logs in the Crownstone firmware. That is not so useful to the
	// microapp person though. To send it through to UART for a USB dongle is quite limited, for normal 
	// Crownstones it is almost useless. It would be fun to write over Bluetooth RFCOMM.
	//
	int write(String str, int length);

	// Write a string (as char array) to serial. The length will be obtained through searching for a null
	// byte.
	int write(const char *str);

	// Write an array of bytes to serial.
	int write(const char *buf, int length);

	// Write a single byte to serial.
	void write(char value);
};

class Serial_: SerialBase_ {
public:
	static Serial_& getInstance()
	{
		// Guaranteed to be destroyed.
		static Serial_ instance(1);
		
		// Instantiated on first use.
		return instance;
	}

private:
	Serial_(char port) : SerialBase_(port) {}
	Serial_(Serial_ const&)         = delete;
	void operator=(Serial_ const&)  = delete;
};

class SerialServiceData_: SerialBase_ {
public:
	static SerialServiceData_& getInstance()
	{
		// Guaranteed to be destroyed.
		static SerialServiceData_ instance(4);
		
		// Instantiated on first use.
		return instance;
	}

private:
	Serial_(char port) : SerialBase_(port) {}
	Serial_(Serial_ const&)         = delete;
	void operator=(Serial_ const&)  = delete;
};

#define Serial Serial_::getInstance()
#define SerialServiceData SerialServiceData_::getInstance()

