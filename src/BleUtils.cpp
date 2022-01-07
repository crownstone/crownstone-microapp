#include <BleUtils.h>

MacAddress::MacAddress(const uint8_t* mac) {
	memcpy(_mac, mac, MAC_ADDRESS_LENGTH);
	convertMacToString(_mac,_mac_str);
	_initialized = true;
}

MacAddress::MacAddress(const char* mac_str) {
	if (strlen(mac_str) != MAC_ADDRESS_STRING_LENGTH) {
		return;
	}
	memcpy(_mac_str, mac_str, MAC_ADDRESS_STRING_LENGTH);
	convertStringToMac(_mac_str,_mac);
	_initialized = true;
}

void MacAddress::convertMacToString(const uint8_t* mac, char* mac_str) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		convertByteToTwoHexChars(mac[i], mac_str + 3 * i);
		if (3 * i + 2 < MAC_ADDRESS_STRING_LENGTH) { // do not add colon after last byte
			mac_str[3 * i + 2] = ':';
		}
	}
}

void MacAddress::convertStringToMac(const char* mac_str, uint8_t* mac) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		mac[i] = convertTwoHexCharsToByte(mac_str + 3 * i);
	}
}

bool MacAddress::isInitialized() {
	return _initialized;
}

String MacAddress::getString() {
	if (!_initialized) {
		return String("XX:XX:XX:XX:XX");
	}
	return String(_mac_str,MAC_ADDRESS_STRING_LENGTH);
}

uint8_t* MacAddress::getBytes() {
	if (!_initialized) {
		return nullptr;
	}
	return _mac;
}



// Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3
uint8_t convertTwoHexCharsToByte(const char* chars) {
	uint8_t val[2] = {0, 0}; // actually two 4-bit values
	for (uint8_t i = 0; i < 2; i++) {
		if (chars[i] >= '0' && chars[i] <= '9') {
			val[i] = chars[i] - '0';
		}
		else if (chars[i] >= 'a' && chars[i] <= 'f') {
			val[i] = chars[i] - 'a' + 10;
		}
		else if (chars[i] >= 'A' && chars[i] <= 'F') {
			val[i] = chars[i] - 'A' + 10;
		}
	}
	// shift most significant 4-bit value 4 bits to the left and add least significant 4-bit value
	return ((val[0] & 0x0F) << 4) | (val[1] & 0x0F);
}

void convertByteToTwoHexChars(uint8_t byte, char* res) {
	uint8_t c[2]; // divide into two 4-bit numbers
	c[0] = (byte >> 4) & 0x0F;
	c[1] = byte & 0x0F;
	for (uint8_t i = 0; i < 2; i++) {
		if (c[i] >= 0 && c[i] <= 9) {
			*res = c[i] + '0';
		}
		else if (c[i] >= 0xA && c[i] <= 0xF) {
			*res = c[i] + 'A' - 10;
		}
		res++;
	}
}


uuid16_t convertStringToUuid(const char* uuid_str) {
	uint8_t byte[2];
	for (uint8_t i = 0; i < 2; i++) {
		byte[i] = convertTwoHexCharsToByte(uuid_str + 2 * i);
	}
	uuid16_t res = (byte[0] << 8) | (byte[1] & 0xFF);
	return res;
}