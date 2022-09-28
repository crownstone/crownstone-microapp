#include <BleMacAddress.h>

MacAddress::MacAddress(const uint8_t* address, uint8_t size, uint8_t type) {
	if (size != MAC_ADDRESS_LENGTH) {
		return;
	}
	memcpy(_address, address, MAC_ADDRESS_LENGTH);
	_type = type;
	_initialized = true;
}

MacAddress::MacAddress(const char* addressString) {
	if (!convertStringToMac(addressString, _address)) {
		return;
	}
	_initialized = true;
}

MacAddress::operator bool() const {
	return this->_initialized;
}

bool MacAddress::operator==(const MacAddress& other) {
	return (memcmp(this->_address, other._address, MAC_ADDRESS_LENGTH) == 0);
}

bool MacAddress::operator!=(const MacAddress& other) {
	return (memcmp(this->_address, other._address, MAC_ADDRESS_LENGTH) != 0);
}

void MacAddress::convertMacToString(const uint8_t* address, char* emptyAddressString) {
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		convertByteToTwoHexChars(address[MAC_ADDRESS_LENGTH - i - 1], emptyAddressString + 3 * i);
		emptyAddressString[3 * i + 2] = ':';
	}
	emptyAddressString[MAC_ADDRESS_STRING_LENGTH] = 0;
}

bool MacAddress::convertStringToMac(const char* addressString, uint8_t* emptyAddress) {
	if (strlen(addressString) != MAC_ADDRESS_STRING_LENGTH) {
		return false;
	}
	for (uint8_t i = 0; i < MAC_ADDRESS_LENGTH; i++) {
		if (!convertTwoHexCharsToByte(addressString + 3 * i, &emptyAddress[MAC_ADDRESS_LENGTH - i - 1])) {
			return false;
		}
	}
	return true;
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