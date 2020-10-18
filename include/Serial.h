#pragma once

#include <String.h>

class Serial_ {
	public:
		static Serial_& getInstance()
		{
			// Guaranteed to be destroyed.
			static Serial_    instance;
			// Instantiated on first use.
			return instance;
		}
	private:
		Serial_() {}                    // Constructor? (the {} brackets) are needed here.

//		Serial_(Serial_ const&);              // Don't Implement
//		void operator=(Serial_ const&);      // Don't implement

	public:
		Serial_(Serial_ const&)          = delete;
		void operator=(Serial_ const&)  = delete;

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

		// Write a string (as char array) to serial.
		int write(const char *str, int length);

		// Write a single byte to serial.
		void write(char value);
};

#define Serial Serial_::getInstance()

