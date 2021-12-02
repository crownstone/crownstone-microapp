#include <microapp.h>

// Utilities for the microapp BLE library

// type for 16-bit uuid
typedef uint16_t uuid16_t;

// MAC address struct
typedef struct {
	uint8_t byte[MAC_ADDRESS_LENGTH];
} MACaddress;

// Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3
uint8_t convertTwoHexCharsToByte(const char* chars);

// Convert a byte (uint8_t) to its hex string representation, e.g. convert 0xA3 to "A3"
void convertByteToTwoHexChars(uint8_t byte, char* res);

// Convert from MAC address string "aa:bb:cc:dd:ee:ff" to MACaddress struct
MACaddress convertStringToMac(const char* mac_str);

// Convert from MACaddress struct to string "aa":bb:cc:dd:ee:ff"
void convertMacToString(MACaddress mac, char* res);

uuid16_t convertStringToUuid(const char* uuid_str);