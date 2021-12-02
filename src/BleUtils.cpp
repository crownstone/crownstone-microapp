#include <BleUtils.h>

// Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3
uint8_t convertTwoHexCharsToByte(const char* chars) {
	uint8_t val[2] = {0, 0}; // actually two 4-bit values
	char* p = (char*) chars;
	for (uint8_t i = 0; i < 2; i++) {
		if (*p >= '0' && *p <= '9') {
			val[i] = *p - '0';
		}
		else if (*p >= 'a' && *p <= 'f') {
			val[i] = *p - 'a' + 10;
		}
		else if (*p >= 'A' && *p <= 'F') {
			val[i] = *p - 'A' + 10;
		}
		p++;
	}
	// shift most significant 4-bit value 4 bits to the left and add least significant 4-bit value
	return ((val[0] & 0x0F) << 4) | (val[1] & 0x0F);
}

void convertByteToTwoHexChars(uint8_t byte, char* res) {
	uint8_t c[2]; // divide into two 4-bit numbers
	c[0] = (byte >> 4) & 0x0F;
	c[1] = byte & 0x0F;
	uint8_t* p = c;
	for (uint8_t i = 0; i < 2; i++) {
		if (*p >= 0 && *p <= 9) {
			*res = *p + '0';
		}
		else if (*p >= 0xA && *p <= 0xF) {
			*res = *p + 'A' - 10;
		}
		p++;
		res++;
	}
}

// Convert from mac address string "aa:bb:cc:dd:ee:ff" to uint8_t array
MACaddress convertStringToMac(const char* mac_str) {
	// initialize return value
	MACaddress mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t len = strlen(mac_str);
	if (len != 17) {  // length of 'stringified' mac address
		return mac;  // input string not of correct size
	}
	char* p = (char*) mac_str;
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		mac.byte[i] = convertTwoHexCharsToByte(p);
		p += 3;  // skip 3 chars and go to the next pair of chars
	}
	return mac;
};

void convertMacToString(MACaddress mac, char* res) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		char c[2] = {'0','0'};
		convertByteToTwoHexChars(mac.byte[i], c);
		res[3*i] = c[0];
		res[3*i+1] = c[1];
		res[3*i+2] = ':';
	}
	// replace last colon with a terminating character
	res[17] = 0;
};

uuid16_t convertStringToUuid(const char* uuid_str) {
	uint8_t byte[2];
	char* p = (char*) uuid_str;
	for (uint8_t i = 0; i < 2; i++) {
		byte[i] = convertTwoHexCharsToByte(p);
		p += 2;
	}
	uuid16_t res = (byte[0] << 8) | (byte[1] & 0xFF);
	return res;
}

bool findAdvType(GapAdvType type, uint8_t* advData, uint8_t advLen, uint8_t* foundData) {
	uint8_t i = 0;
	foundData = nullptr;
	while (i < advLen-1) {
		uint8_t fieldLen = advData[i];
		uint8_t fieldType = advData[i+1];
		if (fieldLen == 0 || i + 1 + fieldLen > advLen) {
			return false;
		}
		if (fieldType == type) {
			foundData = &advData[i];
			return true;
		}
		i += fieldLen+1;
	}
	return false;
}