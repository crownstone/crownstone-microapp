#include <microapp.h>

// Utilities for the microapp BLE library

// type for 16-bit uuid
typedef uint16_t uuid16_t;

// MAC address struct
typedef struct {
	uint8_t byte[MAC_ADDRESS_LENGTH];
} MACaddress;

struct data_ptr_t {
	uint8_t* data = nullptr;
	uint16_t len = 0;
};

// GAP advertisement types, see
// https://www.bluetooth.com/specifications/assigned-numbers/
enum GapAdvType {
	ShortenedLocalName = 0x08,
	CompleteLocalName  = 0x09,
	ServiceData        = 0x16
};

// Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3
uint8_t convertTwoHexCharsToByte(const char* chars);

// Convert a byte (uint8_t) to its hex string representation, e.g. convert 0xA3
// to "A3"
void convertByteToTwoHexChars(uint8_t byte, char* res);

// Convert from MAC address string "aa:bb:cc:dd:ee:ff" to MACaddress struct
MACaddress convertStringToMac(const char* mac_str);

// Convert from MACaddress struct to string "aa":bb:cc:dd:ee:ff"
void convertMacToString(MACaddress mac, char* res);

// Convert from 16-bit UUID string "181A" to uint16_t 0x181A
uuid16_t convertStringToUuid(const char* uuid_str);

// Tries to find an ad of specified GAP ad type and if found returns true and a pointer to its location
bool findAdvType(GapAdvType type, uint8_t* advData, uint8_t advLen, data_ptr_t* foundAd);