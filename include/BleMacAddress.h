#pragma once

#include <BleUtils.h>
#include <microapp.h>

// length of mac address is defined on bluenet side
extern const uint8_t MAC_ADDRESS_LENGTH;
// format "AA:BB:CC:DD:EE:FF"
const microapp_size_t MAC_ADDRESS_STRING_LENGTH = 17;

/*
 * The MacAddress class stores a buffer containing a mac address
 * This class enables e.g. easy comparison of addresses, and getting a string version of the address
 */
class MacAddress {
private:
	bool _initialized = false;

	/**
	 * Convert from address byte array to string.
	 *
	 * @param[in] address             Pointer to a block of data containing the 6 bytes of the MAC address.
	 * @param[out] emptyAddressString Pointer to the string containing the MAC address in the format
	 * "AA:BB:CC:DD:EE:FF".
	 */
	void convertMacToString(const uint8_t* address, char* emptyAddressString);

	/**
	 * Convert from MAC address string to byte array.
	 *
	 * @param[in] addressString Null-terminated string of the format "AA:BB:CC:DD:EE:FF", with either uppercase or
	 * lowercase letters.
	 * @param[out] emptyAddress Pointer to a block of data containing the 6 bytes of the MAC address.
	 * @return true on success
	 * @return false if no valid string conversion could be made
	 */
	bool convertStringToMac(const char* addressString, uint8_t* emptyAddress);

protected:
	uint8_t _address[MAC_ADDRESS_LENGTH];
	uint8_t _type = MICROAPP_SDK_BLE_ADDRESS_RANDOM_STATIC;

public:
	// Constructors: either empty, from an external struct, or from a string
	MacAddress(){};
	MacAddress(const uint8_t* address, uint8_t size, uint8_t type);
	MacAddress(const char* addressString);

	const char* string();
	const uint8_t* bytes();
	const uint8_t type();

	explicit operator bool() const;
	bool operator==(const MacAddress& other);
	bool operator!=(const MacAddress& other);
};