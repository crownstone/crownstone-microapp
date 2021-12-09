#include <BleUtils.h>

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

// Convert from mac address string "aa:bb:cc:dd:ee:ff" to uint8_t array
MACaddress convertStringToMac(const char* mac_str) {
	// initialize return value
	MACaddress mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t len = strlen(mac_str);
	if (len != MAC_ADDRESS_STRING_LENGTH) {
		return mac;  // input string not of correct size
	}
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		mac.byte[i] = convertTwoHexCharsToByte(mac_str + 3 * i);
	}
	return mac;
};

void convertMacToString(MACaddress mac, char* res) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		convertByteToTwoHexChars(mac.byte[i], res + 3 * i);
		if (3 * i + 2 < MAC_ADDRESS_STRING_LENGTH) { // do not add colon after last byte
			res[3 * i + 2] = ':';
		}
	}
	// replace last colon with a terminating character
	res[MAC_ADDRESS_STRING_LENGTH] = 0;
};

uuid16_t convertStringToUuid(const char* uuid_str) {
	uint8_t byte[2];
	for (uint8_t i = 0; i < 2; i++) {
		byte[i] = convertTwoHexCharsToByte(uuid_str + 2 * i);
	}
	uuid16_t res = (byte[0] << 8) | (byte[1] & 0xFF);
	return res;
}

bool findAdvType(GapAdvType type, uint8_t* advData, uint8_t advLen, data_ptr_t* foundData) {
	uint8_t i = 0;
	foundData->data = nullptr;
	foundData->len = 0;
	while (i < advLen-1) {
		uint8_t fieldLen = advData[i];
		uint8_t fieldType = advData[i+1];
		if (fieldLen == 0 || i + 1 + fieldLen > advLen) {
			return false;
		}
		if (fieldType == type) {
			foundData->data = &advData[i+2];
			foundData->len = fieldLen-1;
			return true;
		}
		i += fieldLen+1;
	}
	return false;
}