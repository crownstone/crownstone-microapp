#pragma once

#include <microapp.h>
#include <String.h>

// UTILITIES FOR THE MICROAPP BLE LIBRARY

// length of mac address is defined on bluenet side
extern const uint8_t MAC_ADDRESS_LENGTH;
// length of 'stringified' mac address of format "AA:BB:CC:DD:EE:FF"
const uint8_t MAC_ADDRESS_STRING_LENGTH = 17;

// The MacAddress class stores both a string version of the mac address and and its 'raw' form as 6 bytes.
// This allows e.g. for easy comparison of mac addresses, and getting a string version of the mac address
class MacAddress {

private:
	bool _initialized = false;
	char _mac_str[MAC_ADDRESS_STRING_LENGTH];
	/**
	 * Convert from MAC address byte array to string.
	 *
	 * @param[in] mac         Pointer to a block of data containing the 6 bytes of the MAC address.
	 * @param[out] mac_str    Pointer to the string containing the MAC address in the format "AA:BB:CC:DD:EE:FF".
	 */
	void convertMacToString(const uint8_t* mac, char* mac_str);

	/**
	 * Convert from MAC address string to byte array.
	 *
	 * @param[in] mac_str   Null-terminated string of the format "AA:BB:CC:DD:EE:FF", with either uppercase or lowercase letters.
	 * @param[out] mac      Pointer to a block of data containing the 6 bytes of the MAC address.
	 */
	void convertStringToMac(const char* mac_str, uint8_t* mac);

protected:
	uint8_t _mac[MAC_ADDRESS_LENGTH];

public:
	MacAddress() {};
	MacAddress(const uint8_t* mac);
	MacAddress(const char* mac_str);

	bool isInitialized();

	String getString();
	uint8_t* getBytes();

	bool operator==(const MacAddress& other) { return memcmp(this->_mac,other._mac,MAC_ADDRESS_LENGTH) == 0; }
	bool operator!=(const MacAddress& other) { return memcmp(this->_mac,other._mac,MAC_ADDRESS_LENGTH) != 0; }

};

// type for 16-bit uuid
typedef uint16_t uuid16_t;

/**
 * Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3.
 *
 * @param[in] chars   Pointer to a pair of chars to convert to a byte.
 *
 * @return            The byte as a uint8_t.
 */
uint8_t convertTwoHexCharsToByte(const char* chars);

/**
 * Convert a byte (uint8_t) to its hex string representation, e.g. convert 0xA3 to "A3".
 *
 * @param[in] byte   Byte to be converted to a pair of chars.
 * @param[out] res   Pointer to a pair of chars.
 */
void convertByteToTwoHexChars(uint8_t byte, char* res);

/**
 * Convert from 16-bit UUID string "180D" to uint16_t 0x180D.
 *
 * @param[in] uuid_str   Null-terminated string of the format "180D", with either uppercase or lowercase letters.
 *
 * @return               A 16-bit UUID int of the format 0x180D.
 */
uuid16_t convertStringToUuid(const char* uuid_str);