#pragma once

#include <String.h>
#include <microapp.h>

/* UTILITIES FOR THE MICROAPP BLE LIBRARY
 * - BLE enums
 * - MAC address class
 * - UUID class
 * - conversion between hex and chars
 */

// Incomplete list of GAP advertisement types, see
// https://www.bluetooth.com/specifications/assigned-numbers/
enum GapAdvType {
	Flags                            = 0x01,
	IncompleteList16BitServiceUuids  = 0x02,
	CompleteList16BitServiceUuids    = 0x03,
	IncompleteList32BitServiceUuids  = 0x04,
	CompleteList32BitServiceUuids    = 0x05,
	IncompleteList128BitServiceUuids = 0x06,
	CompleteList128BitServiceUuids   = 0x07,
	ShortenedLocalName               = 0x08,
	CompleteLocalName                = 0x09,
	ServiceData16BitUuid             = 0x16,
	ServiceData32BitUuid             = 0x20,
	ServiceData128BitUuid            = 0x21,
	ManufacturerSpecificData         = 0xFF,
};

// Types of BLE event for which event handlers can be set
// The naming of these corresponds with ArduinoBLE syntax
enum BleEventType {
	// BleDevice
	BLEDeviceScanned = 0x01,
	BLEConnected     = 0x02,
	BLEDisconnected  = 0x03,
	// BleCharacteristic
	BLESubscribed    = 0x04,
	BLEUnsubscribed  = 0x05,
	BLERead          = 0x06,
	BLEWritten       = 0x07,
};

typedef int8_t rssi_t;

// length of mac address is defined on bluenet side
extern const uint8_t MAC_ADDRESS_LENGTH;
// format "AA:BB:CC:DD:EE:FF"
const size_t MAC_ADDRESS_STRING_LENGTH = 17;

/*
 * The MacAddress class stores a buffer containing a mac address
 * This class enables e.g. easy comparison of addresses, and getting a string version of the address
 */
class MacAddress {
private:
	bool _initialized         = false;

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
	 */
	void convertStringToMac(const char* addressString, uint8_t* emptyAddress);

protected:
	uint8_t _address[MAC_ADDRESS_LENGTH];

public:
	// Constructors: either empty, from an external struct, or from a string
	MacAddress(){};
	MacAddress(const uint8_t* address);
	MacAddress(const char* addressString);

	const char* string();
	const uint8_t* bytes();

	explicit operator bool() const { return this->_initialized; }
	bool operator==(const MacAddress& other) { return memcmp(this->_address, other._address, MAC_ADDRESS_LENGTH) == 0; }
	bool operator!=(const MacAddress& other) { return memcmp(this->_address, other._address, MAC_ADDRESS_LENGTH) != 0; }
};

// type for 16-bit uuid
typedef uint16_t uuid16_t;
// amount of bytes in a 16-bit uuid
const size_t UUID_16BIT_BYTE_LENGTH = 2;
// format "ABCD"
const size_t UUID_16BIT_STRING_LENGTH = 4;
// amount of bytes in a 128-bit uuid
const size_t UUID_128BIT_BYTE_LENGTH = 16;
// format "12345678-ABCD-1234-5678-ABCDEF123456"
const size_t UUID_128BIT_STRING_LENGTH = 36;

class Uuid {
private:

	static constexpr uint8_t BASE_UUID_128BIT[UUID_128BIT_BYTE_LENGTH] = {0xFB, 0x34, 0x9B, 0x5F,
		0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static constexpr uint8_t BASE_UUID_OFFSET_16BIT = 12;

	uint8_t _uuid[UUID_128BIT_BYTE_LENGTH];
	uint8_t _length = 0;
	// after registration, bluenet passes an id for custom uuids
	uint8_t _type = 0;
	bool _initialized = false;

	/**
	 * Convert from 16-bit UUID string "ABCD" to 16-bit byte array
	 *
	 * @param[in] uuidString Null-terminated string of the format "ABCD", with either uppercase or lowercase letters.
	 * @param[out] emptyUuid A byte array where the resulting UUID will be placed
	 */
	void convertStringToUuid16Bit(const char* uuidString, uint8_t* emptyUuid);

	/**
	 * Convert from UUID string (either 16-bit or 128-bit) to 128-bit byte array
	 *
	 * @param[in] uuidString Null-terminated string of the format "12345678-ABCD-1234-5678-ABCDEF123456", with either uppercase or lowercase letters.
	 * @param[out] emptyUuid A byte array where the resulting UUID will be placed
	 */
	void convertStringToUuid128Bit(const char* uuidString, uint8_t* emptyUuid);

	/**
	 * Convert from 16-bit UUID to string representation in format "ABCD"
	 *
	 * @param[in] uuid             Pointer to byte array containing 16-bit UUID
	 * @param[out] emptyUuidString Char array where the stringified UUID will be placed
	 */
	void convertUuid16BitToString(const uint8_t* uuid, char* emptyUuidString);

	/**
	 * Convert from 128-bit UUID to string representation in format "12345678-ABCD-1234-5678-ABCDEF123456"
	 *
	 * @param[in] uuid             Pointer to byte array containing 128-bit UUID
	 * @param[out] emptyUuidString Char array where the stringified UUID will be placed
	 */
	void convertUuid128BitToString(const uint8_t* uuid, char* emptyUuidString);

public:
	Uuid(){};
	Uuid(const char* uuid);
	Uuid(const uint8_t* uuid, uint8_t length);
	Uuid(const uuid16_t uuid);

	// boolean and comparison operators
	explicit operator bool() const { return this->_initialized; }
	bool operator==(const Uuid& other) { return memcmp(this->_uuid, other._uuid, UUID_128BIT_BYTE_LENGTH) == 0; }
	bool operator!=(const Uuid& other) { return memcmp(this->_uuid, other._uuid, UUID_128BIT_BYTE_LENGTH) != 0; }

	// even though internally it's always 16 bytes, the length can be either 2 or 16
	uint8_t length();
	bool custom();

	const char* string();
	// return full string, even for 16-bit uuids
	const char* fullString();

	const uint8_t* bytes();
	const uint8_t* fullBytes();

	// makes sense only for 16-bit uuids! Returns a 16-bit int version of the uuid
	uuid16_t uuid16();

	void setCustomId(uint8_t customId);
	uint8_t getType();

};

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
