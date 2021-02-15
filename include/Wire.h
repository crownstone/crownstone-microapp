#pragma once

#include <String.h>
#include <stdint.h>
#include <microapp.h>

#define WIRE_SIZE_OPCODE                           6
#define WIRE_MAX_PAYLOAD_LENGTH                    (MAX_PAYLOAD - WIRE_SIZE_OPCODE)
#define WIRE_MAX_STRING_LENGTH                     (WIRE_MAX_PAYLOAD_LENGTH - 1)

class WireBase_ {
private:
	// TODO: can be removed
	char _port;

	// The address of the twi device to talk to (7 bits)
	uint8_t _address;

	// Extra buffer for incoming data
	uint8_t _readBuf[WIRE_MAX_PAYLOAD_LENGTH];

	// How much data is read
	int8_t _readLen;

	// Current pointer
	int8_t _readPtr;
protected:
	WireBase_(char port) : _port(port) {}

	enum Type { Char = 0, Int = 1, Str = 2, Arr = 3};
	
	// Write an array of bytes to i2c.
	// Returns number of bytes written.
	int _write(const uint8_t *buf, int length, Type type);

public:

	// Start as slave
	// The address is your own address.
	// TODO: not supported yet
	// void begin(const uint8_t address);
	
	// Start as master
	void begin();

	// Returns always true for now.
	// Can be used as
	//   if(Wire)
	// Should return false if i2c is not enabled.
	explicit operator bool() const { return true; }

	// Start a transmission
	// The address is the address of the device you will send to.
	void beginTransmission(const uint8_t address);
	
	// Stop a transmission
	void endTransmission();

	// Request a number of bytes from a slave device
	// The boolean stop indicates if the bus needs to be released
	void requestFrom(const uint8_t address, const int size, bool stop = false);

	// TODO: register callback and call from bluenet (this is as slave device)
	// void onReceive(void *receiveEvent);

	// Returns if there are bytes available
	int available();

	// Returns a single byte
	const uint8_t read();

	// Write to i2c port.
	//
	// Returns number of bytes written.
	int write(String str, int length);

	// Write a string (as char array) to serial. The length will be obtained through searching for a null
	// byte.
	// Returns number of bytes written.
	int write(const char *str);

	// Write an array of bytes to serial.
	// Returns number of bytes written.
	int write(const uint8_t *buf, int length);

	// Write a single byte to serial.
	// Returns number of bytes written.
	int write(char value);

	// Wire has both write and send defined
	int send(String str, int length);
	int send(const char *str);
	int send(const uint8_t *buf, int length);
	int send(char value);
};

class Wire_: public WireBase_ {
public:
	static Wire_& getInstance()
	{
		// Guaranteed to be destroyed.
		static Wire_ instance(1);

		// Instantiated on first use.
		return instance;
	}

private:
	Wire_(char port) : WireBase_(port) {}
	Wire_(Wire_ const&)         = delete;
	void operator=(Wire_ const&)  = delete;
};

#define Wire Wire_::getInstance()

