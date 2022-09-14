#include <BleUtils.h>

MacAddress::MacAddress(const uint8_t* address) {
	memcpy(_address, address, MAC_ADDRESS_LENGTH);
	_initialized = true;
}

MacAddress::MacAddress(const char* addressString) {
	if (strlen(addressString) != MAC_ADDRESS_STRING_LENGTH) {
		return;
	}
	convertStringToMac(addressString, _address);
	_initialized = true;
}

void MacAddress::convertMacToString(const uint8_t* address, char* emptyAddressString) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		convertByteToTwoHexChars(address[MAC_ADDRESS_LENGTH - i - 1], emptyAddressString + 3 * i);
		emptyAddressString[3 * i + 2] = ':';
	}
	emptyAddressString[MAC_ADDRESS_STRING_LENGTH] = 0;
}

void MacAddress::convertStringToMac(const char* addressString, uint8_t* emptyAddress) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		emptyAddress[MAC_ADDRESS_LENGTH - i - 1] = convertTwoHexCharsToByte(addressString + 3 * i);
	}
}

const char* MacAddress::string() {
	if (!_initialized) {
		return nullptr;
	}
	static char addressString[MAC_ADDRESS_STRING_LENGTH + 1];
	convertMacToString(_address, addressString);
	return addressString;
}

const uint8_t* MacAddress::bytes() {
	if (!_initialized) {
		return nullptr;
	}
	return _address;
}

Uuid16Bit::Uuid16Bit(const char* uuidString) {
	if (strlen(uuidString) != UUID_16BIT_STRING_LENGTH) {
		return;
	}
	_uuid = convertStringToUuid(uuidString);
	_initialized = true;
}

Uuid16Bit::Uuid16Bit(uuid16_t uuid) {
	_uuid = uuid;
	_initialized = true;
}

uuid16_t Uuid16Bit::convertStringToUuid(const char* uuidString) {
	uint8_t byte[UUID_16BIT_BYTE_LENGTH];
	for (uint8_t i = 0; i < UUID_16BIT_BYTE_LENGTH; i++) {
		byte[i] = convertTwoHexCharsToByte(uuidString + 2 * i);
	}
	uuid16_t res = (byte[0] << 8) | (byte[1] & 0xFF);
	return res;
}

void Uuid16Bit::convertUuidToString(const uuid16_t uuid, char* emptyUuidString) {
	uint8_t msb = ((uuid >> 8) & 0xFF);
	uint8_t lsb = (uuid & 0xFF);
	convertByteToTwoHexChars(msb, emptyUuidString);
	convertByteToTwoHexChars(lsb, emptyUuidString + 2);
	// string termination
	emptyUuidString[UUID_16BIT_STRING_LENGTH] = 0;
}

uuid16_t Uuid16Bit::uuid() {
	if (!_initialized) {
		return 0;
	}
	return _uuid;
}

const char* Uuid16Bit::string() {
	if (!_initialized) {
		return nullptr;
	}
	static char uuidString[UUID_16BIT_STRING_LENGTH + 1];
	convertUuidToString(_uuid, uuidString);
	return uuidString;
}

// Can be initialized with either 16-bit or 128-bit uuid strings
Uuid128Bit::Uuid128Bit(const char* uuidString) {
	if (strlen(uuidString) == UUID_128BIT_STRING_LENGTH) {
		convertStringToUuid(uuidString, _uuid);
	}
	else if (strlen(uuidString) == UUID_16BIT_STRING_LENGTH) {
		memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
		_uuid[CUSTOM_UUID_BYTE_OFFSET] = convertTwoHexCharsToByte(uuidString);
		_uuid[CUSTOM_UUID_BYTE_OFFSET + 1] = convertTwoHexCharsToByte(uuidString + 2);
	}
	else {
		return;
	}
	_initialized = true;
}

Uuid128Bit::Uuid128Bit(const uint8_t* uuid, size_t len) {
	if (len != UUID_128BIT_BYTE_LENGTH) {
		return;
	}
	memcpy(_uuid, uuid, len);
	_initialized = true;
}

Uuid128Bit::Uuid128Bit(uuid16_t uuid) {
	// first use the base uuid
	memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
	// overwrite the two custom uuid bytes with the provided uuid
	uint8_t byte[UUID_16BIT_BYTE_LENGTH];
	byte[0] = ((uuid >> 8) & 0xFF);
	byte[1] = (uuid & 0xFF);
	memcpy(_uuid + CUSTOM_UUID_BYTE_OFFSET, byte, UUID_16BIT_BYTE_LENGTH);
	_initialized = true;
}

void Uuid128Bit::convertStringToUuid(const char* uuidString, uint8_t* emptyUuid) {
	if (strlen(uuidString) != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	uint8_t i = 0;
	uint8_t j = 0;
	while (uuidString[i] != 0) {
		if (uuidString[i] == '-') {
			i++;
			continue;
		}
		emptyUuid[j] = convertTwoHexCharsToByte(&uuidString[i]);
		j++;
		i += 2;
	}
}

void Uuid128Bit::convertUuidToString(const uint8_t* uuid, char* emptyUuidString) {
	uint8_t j = 0;
	for (uint8_t i = 0; i < UUID_128BIT_BYTE_LENGTH; i++) {
		convertByteToTwoHexChars(uuid[i], &emptyUuidString[j]);
		j += 2;
		if (j == 8 || j == 13 || j == 18 || j == 23) {
			emptyUuidString[j] = '-';
			j++;
		}
	}
	emptyUuidString[UUID_128BIT_STRING_LENGTH] = 0;
}

const uint8_t* Uuid128Bit::bytes() {
	if (!_initialized) {
		return 0;
	}
	return _uuid;
}

const char* Uuid128Bit::string() {
	if (!_initialized) {
		return nullptr;
	}
	static char uuidString[UUID_128BIT_STRING_LENGTH + 1];
	convertUuidToString(_uuid, uuidString);
	return uuidString;
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
