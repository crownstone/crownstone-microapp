#include <BleUtils.h>

MacAddress::MacAddress(const uint8_t* address) {
	memcpy(_address, address, MAC_ADDRESS_LENGTH);
	_initialized = true;
}

MacAddress::MacAddress(const char* addressString) {
	if (strlen(addressString) != MAC_ADDRESS_STRING_LENGTH - 1) {
		return;
	}
	memcpy(_addressString, addressString, MAC_ADDRESS_STRING_LENGTH);
	_cachedAddressString = true;
	convertStringToMac(_addressString, _address);
	_initialized = true;
}

void MacAddress::convertMacToString(const uint8_t* address, char* emptyAddressString) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		convertByteToTwoHexChars(address[i], emptyAddressString + 3 * i);
		if (3 * i + 2 < MAC_ADDRESS_STRING_LENGTH) {  // do not add colon after last byte
			emptyAddressString[3 * i + 2] = ':';
		}
	}
	// string termination
	emptyAddressString[MAC_ADDRESS_STRING_LENGTH-1] = 0;
}

void MacAddress::convertStringToMac(const char* addressString, uint8_t* emptyAddress) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		emptyAddress[i] = convertTwoHexCharsToByte(addressString + 3 * i);
	}
}

const char* MacAddress::string() {
	if (!_initialized) {
		return nullptr;
	}
	if (!_cachedAddressString) {
		convertMacToString(_address, _addressString);
		_cachedAddressString = true;
	}
	return _addressString;
}

const uint8_t* MacAddress::bytes() {
	if (!_initialized) {
		return nullptr;
	}
	return _address;
}

// Convert a pair of chars to a byte, e.g. convert "A3" to 0xA3
uint8_t convertTwoHexCharsToByte(const char* chars) {
	uint8_t val[2] = {0, 0};  // actually two 4-bit values
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
	uint8_t c[2];  // divide into two 4-bit numbers
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