#include <BleUuid.h>

Uuid::Uuid(const char* uuid) {
	if (strlen(uuid) == UUID_128BIT_STRING_LENGTH) {
		if (!convertStringToUuid128Bit(uuid, _uuid)) {
			return;
		}
		_length = UUID_128BIT_BYTE_LENGTH;
	}
	else if (strlen(uuid) == UUID_16BIT_STRING_LENGTH) {
		memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
		if (!convertStringToUuid16Bit(uuid, _uuid + BASE_UUID_OFFSET_16BIT)) {
			return;
		}
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

Uuid::Uuid(const uuid16_t uuid, uint8_t type) {
	memcpy(_uuid, BASE_UUID_128BIT, UUID_128BIT_BYTE_LENGTH);
	_uuid[BASE_UUID_OFFSET_16BIT] = uuid & 0xFF;
	_uuid[BASE_UUID_OFFSET_16BIT + 1] = (uuid >> 8) & 0xFF;
	_length = UUID_16BIT_BYTE_LENGTH;
	_type = type;
	_initialized = true;
}

bool Uuid::operator==(const Uuid& other) {
	return (memcmp(this->_uuid, other._uuid, UUID_128BIT_BYTE_LENGTH) == 0);
}

bool Uuid::operator!=(const Uuid& other) {
	return (memcmp(this->_uuid, other._uuid, UUID_128BIT_BYTE_LENGTH) != 0);
}

bool Uuid::registered() {
	return (_type != CS_MICROAPP_SDK_BLE_UUID_NONE);
}

microapp_sdk_result_t Uuid::registerCustom() {
	if (registered()) {
		// apparently already registered so just return success
		return CS_MICROAPP_SDK_ACK_SUCCESS;
	}
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_UUID_REGISTER;
	memcpy(bleRequest->requestUuidRegister.customUuid, _uuid, UUID_128BIT_BYTE_LENGTH);

	sendMessage();
	microapp_sdk_result_t result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	if (memcmp(&bleRequest->requestUuidRegister.uuid.uuid, _uuid + BASE_UUID_OFFSET_16BIT, UUID_16BIT_BYTE_LENGTH) != 0) {
		// The returned short uuid is not the same as the original
		// (it should be the same)
		return CS_MICROAPP_SDK_ACK_ERROR;
	}
	_type = bleRequest->requestUuidRegister.uuid.type;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

uint8_t Uuid::length() {
	return _length;
}

bool Uuid::custom() {
	return (_length == UUID_128BIT_BYTE_LENGTH);
}

bool Uuid::valid() {
	return _initialized;
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

void Uuid::setType(uint8_t type) {
	_type = type;
}

uint8_t Uuid::getType() {
	return _type;
}

bool Uuid::convertStringToUuid16Bit(const char* uuidString, uint8_t* emptyUuid) {
	if (strlen(uuidString) != UUID_16BIT_STRING_LENGTH) {
		return false;
	}
	for (uint8_t i = 0; i < UUID_16BIT_BYTE_LENGTH; i++) {
		if (!convertTwoHexCharsToByte(uuidString + 2 * i, &emptyUuid[UUID_16BIT_BYTE_LENGTH - 1 - i])) {
			return false;
		}
	}
	return true;
}

bool Uuid::convertStringToUuid128Bit(const char* uuidString, uint8_t* emptyUuid) {
	if (strlen(uuidString) != UUID_128BIT_STRING_LENGTH) {
		return false;
	}
	uint8_t i = 0;
	int8_t j = UUID_128BIT_BYTE_LENGTH - 1;
	while (j >= 0) {
		if (uuidString[i] == '-') {
			i++;
			continue;
		}
		if (i + 1 > UUID_128BIT_STRING_LENGTH) {
			return false;
		}
		if (!convertTwoHexCharsToByte(&uuidString[i], &emptyUuid[j])) {
			return false;
		}
		j--;
		i += 2;
	}
	return true;
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