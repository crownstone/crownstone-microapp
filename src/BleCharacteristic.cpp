#include <BleCharacteristic.h>

BleCharacteristic::BleCharacteristic(const char* uuid, uint8_t properties, bool remote) {
	size_t len = strlen(uuid);
	if (len != UUID_16BIT_STRING_LENGTH && len != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	if (len == UUID_128BIT_STRING_LENGTH) {
		_flags.flags.vendorSpecific = true;
	}
	_uuid        = Uuid(uuid);
	_properties  = properties;
	_remote      = remote;
	_initialized = true;
}

// Outside the Ble/BleService class, only this constructor (and the empty) can be called
// remote = false means the characteristic is local (crownstone = peripheral)
BleCharacteristic::BleCharacteristic(const char* uuid, uint8_t properties)
		: BleCharacteristic(uuid, properties, false) {}

String BleCharacteristic::uuid() {
	if (!_initialized) {
		return String(nullptr);
	}
	return String(_uuid.string());
}

uint8_t BleCharacteristic::properties() {
	return _properties;
}

int BleCharacteristic::valueSize() {
	return MAX_CHARACTERISTIC_VALUE_SIZE;
}

uint8_t* BleCharacteristic::value() {
	if (!_initialized) {
		return nullptr;
	}
	return _value;
}

int BleCharacteristic::valueLength() {
	return _valueLength;
}

bool BleCharacteristic::writeValue(uint8_t* buffer, size_t length) {
	// todo: check if local or remote write
	if (length > MAX_CHARACTERISTIC_VALUE_SIZE) {
		length = MAX_CHARACTERISTIC_VALUE_SIZE;
	}
	_value       = buffer;
	_valueLength = length;
	// todo: check for notify, send peripheral_value_set
	return true;
}

void BleCharacteristic::setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice, BleCharacteristic)) {
	// Only defined for peripheral role
	if (_remote) {
		return;
	}
	// todo: implement
}

bool BleCharacteristic::written() {
	// Only defined for peripheral role
	if (_remote) {
		return false;
	}
	return _written;
}

bool BleCharacteristic::subscribed() {
	// Only defined for peripheral role
	if (_remote) {
		return false;
	}
	return _subscribed;
}
