#pragma once

#include <microapp.h>

// Utilities for the microapp BLE library

// type for 16-bit uuid
typedef uint16_t uuid16_t;

// length of 'stringified' mac address
const uint8_t MAC_ADDRESS_STRING_LENGTH = 17;

// MAC address struct
typedef struct {
	uint8_t byte[MAC_ADDRESS_LENGTH];
} MACaddress;

// struct containing a pointer to a block of data, and a length field to indicate the length of the block
struct data_ptr_t {
	uint8_t* data = nullptr;
	size_t len = 0;
};

// GAP advertisement types, see
// https://www.bluetooth.com/specifications/assigned-numbers/
enum GapAdvType {
	CompleteList16BitServiceUuids  = 0x03,
	CompleteList128BitServiceUuids = 0x07,
	ShortenedLocalName             = 0x08,
	CompleteLocalName              = 0x09,
	ServiceData                    = 0x16,
	ManufacturerSpecificData       = 0xFF
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

/**
 * Convert from MAC address string to MACaddress struct.
 *
 * @param[in] mac_str   Null-terminated string of the format "aa:bb:cc:dd:ee:ff", with either uppercase or lowercase letters.
 *
 * @return              A MACaddress struct containing the 6 bytes of the MAC address.
 */
MACaddress convertStringToMac(const char* mac_str);

/**
 * Convert from MACaddress struct to string.
 *
 * @param[in] mac    MACaddress struct containing the 6 bytes of the MAC address.
 * @param[out] res   Pointer to the string containing the MAC address in the format "AA:BB:CC:DD:EE:FF".
 */
void convertMacToString(MACaddress mac, char* res);

/**
 * Convert from 16-bit UUID string "180D" to uint16_t 0x180D.
 *
 * @param[in] uuid_str   Null-terminated string of the format "180D", with either uppercase or lowercase letters.
 *
 * @return               A 16-bit UUID int of the format 0x180D.
 */
uuid16_t convertStringToUuid(const char* uuid_str);

//
/**
 * Tries to find an ad of specified GAP ad type. and if found returns true and a pointer to its location.
 *
 * @param[in] type          GAP advertisement type according to https://www.bluetooth.com/specifications/assigned-numbers/.
 * @param[in] advData       Pointer to the raw advertisement data.
 * @param[in] advLen        Length of advData in bytes.
 * @param[out] foundAd      data_ptr_t containing a pointer to the first byte of advData containing the data of type type and its length.
 *
 * @return true             if the advertisement data of given type is found in advData.
 * @return false            if the advertisement data of given type is not found in advData.
 */
bool findAdvType(GapAdvType type, uint8_t* advData, uint8_t advLen, data_ptr_t* foundAd);