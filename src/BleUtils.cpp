#include <BleUtils.h>

MacAddress::MacAddress(const uint8_t* address, uint8_t type) {
	memcpy(_address, address, MAC_ADDRESS_LENGTH);
	_type = type;
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

const uint8_t MacAddress::type() {
	return _type;
}

////////////////////////////////////////////////////////////////////////

Uuid::Uuid(const char* uuid) {
	if (strlen(uuid) == UUID_128BIT_STRING_LENGTH) {
		convertStringToUuid128Bit(uuid, _uuid);
		_length = UUID_128BIT_BYTE_LENGTH;
	}
	else if (strlen(uuid) == UUID_16BIT_STRING_LENGTH) {
		memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
		convertStringToUuid16Bit(uuid, _uuid + BASE_UUID_OFFSET_16BIT);
		_length = UUID_16BIT_BYTE_LENGTH;
		_type = CS_MICROAPP_SDK_BLE_UUID_STANDARD;
	}
	else {
		return;
	}
	_initialized = true;
}

Uuid::Uuid(const uint8_t* uuid, uint8_t length) {
	if (length == UUID_128BIT_BYTE_LENGTH) {
		memcpy(_uuid, uuid, length);
		_length = UUID_128BIT_BYTE_LENGTH;
	}
	else if (length == UUID_16BIT_BYTE_LENGTH) {
		memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
		memcpy(_uuid + BASE_UUID_OFFSET_16BIT, uuid, length);
		_length = UUID_16BIT_BYTE_LENGTH;
		_type = CS_MICROAPP_SDK_BLE_UUID_STANDARD;
	}
	else {
		return;
	}
	_initialized = true;
}

Uuid::Uuid(const uuid16_t uuid) {
	memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
	_uuid[BASE_UUID_OFFSET_16BIT] = uuid & 0xFF;
	_uuid[BASE_UUID_OFFSET_16BIT + 1] = (uuid >> 8) & 0xFF;
	_length = UUID_16BIT_BYTE_LENGTH;
	_type = CS_MICROAPP_SDK_BLE_UUID_STANDARD;
	_initialized = true;
}

uint8_t Uuid::length() {
	return _length;
}

bool Uuid::custom() {
	return (_length == UUID_128BIT_BYTE_LENGTH);
}

const char* Uuid::string() {
	if (!_initialized) {
		return nullptr;
	}
	if (_length == UUID_128BIT_BYTE_LENGTH) {
		static char uuidString128[UUID_128BIT_STRING_LENGTH + 1];
		convertUuid128BitToString(_uuid, uuidString128);
		return uuidString128;
	}
	else if (_length == UUID_16BIT_BYTE_LENGTH) {
		static char uuidString16[UUID_16BIT_STRING_LENGTH + 1];
		convertUuid16BitToString(_uuid + BASE_UUID_OFFSET_16BIT, uuidString16);
		return uuidString16;
	}
	else {
		return nullptr;
	}
}

const char* Uuid::fullString() {
	if (!_initialized) {
		return nullptr;
	}
	if (_length != UUID_128BIT_BYTE_LENGTH && _length != UUID_16BIT_BYTE_LENGTH) {
		return nullptr;
	}
	static char uuidString128[UUID_128BIT_STRING_LENGTH + 1];
	convertUuid128BitToString(_uuid, uuidString128);
	return uuidString128;
}

const uint8_t* Uuid::bytes() {
	if (!_initialized) {
		return nullptr;
	}
	if (_length == UUID_16BIT_BYTE_LENGTH) {
		return _uuid + BASE_UUID_OFFSET_16BIT;
	}
	return _uuid;
}

const uint8_t* Uuid::fullBytes() {
	if (!_initialized) {
		return nullptr;
	}
	return _uuid;
}

uuid16_t Uuid::uuid16() {
	return (_uuid[BASE_UUID_OFFSET_16BIT + 1] << 8) | (_uuid[BASE_UUID_OFFSET_16BIT] & 0xFF);
}

void Uuid::setCustomId(uint8_t customId) {
	_type = customId;
}

uint8_t Uuid::getType() {
	return _type;
}

void Uuid::convertStringToUuid16Bit(const char* uuidString, uint8_t* emptyUuid) {
	if (strlen(uuidString) != UUID_16BIT_STRING_LENGTH) {
		return;
	}
	for (uint8_t i = 0; i < UUID_16BIT_BYTE_LENGTH; i++) {
		emptyUuid[UUID_16BIT_BYTE_LENGTH - 1 - i] = convertTwoHexCharsToByte(uuidString + 2 * i);
	}
}

void Uuid::convertStringToUuid128Bit(const char* uuidString, uint8_t* emptyUuid) {
	if (strlen(uuidString) != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	uint8_t i = 0;
	uint8_t j = UUID_128BIT_BYTE_LENGTH - 1;
	while (uuidString[i] != 0) {
		if (uuidString[i] == '-') {
			i++;
			continue;
		}
		emptyUuid[j] = convertTwoHexCharsToByte(&uuidString[i]);
		j--;
		i += 2;
	}
}

void Uuid::convertUuid16BitToString(const uint8_t* uuid, char* emptyUuidString) {
	for (uint8_t i = 0; i < UUID_16BIT_BYTE_LENGTH; i++) {
		convertByteToTwoHexChars(*(uuid + UUID_16BIT_BYTE_LENGTH - 1 - i), emptyUuidString + 2 * i);
	}
	emptyUuidString[UUID_16BIT_STRING_LENGTH] = 0;
}

void Uuid::convertUuid128BitToString(const uint8_t* uuid, char* emptyUuidString) {
	uint8_t j = 0;
	for (uint8_t i = 0; i < UUID_128BIT_BYTE_LENGTH; i++) {
		convertByteToTwoHexChars(uuid[UUID_128BIT_BYTE_LENGTH - 1 - i], &emptyUuidString[j]);
		j += 2;
		if (j == 8 || j == 13 || j == 18 || j == 23) {
			emptyUuidString[j] = '-';
			j++;
		}
	}
	emptyUuidString[UUID_128BIT_STRING_LENGTH] = 0;
}

/////////////////////////////////////////////////////////////////////////

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