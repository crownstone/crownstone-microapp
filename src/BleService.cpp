#include <BleService.h>

BleService::BleService(const char* uuid, bool remote) {
	size_t len = strlen(uuid);
	if (len != UUID_16BIT_STRING_LENGTH && len != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	if (len == UUID_128BIT_STRING_LENGTH) {
		_customUuid = true;
	}
	_uuid = Uuid(uuid);
	_remote = remote;
	_initialized = true;
}

// Outside the Ble class, only this constructor (and the empty) can be called
// remote = false means the service is local (crownstone = peripheral)
BleService::BleService(const char* uuid) : BleService(uuid, false){}

String BleService::uuid() {
	if (!_initialized) {
		return String(nullptr);
	}
	return String(_uuid.string());
}

void BleService::addCharacteristic(BleCharacteristic& characteristic) {
	if (_characteristicCount >= MAX_CHARACTERISTICS) {
		return;
	}
	_characteristics[_characteristicCount] = &characteristic;
	_characteristicCount++;
}

int BleService::characteristicCount() {
	return _characteristicCount;
}

bool BleService::hasCharacteristic(const char* uuidString) {
	Uuid uuid(uuidString);
	for (int i = 0; i < _characteristicCount; i++) {
		if (_characteristics[i]->_uuid == uuid) {
			return true;
		}
	}
	return false;
}

// Returns a copy of the characteristic
BleCharacteristic BleService::characteristic(const char* uuidString) {
	Uuid uuid(uuidString);
	for (int i = 0; i < _characteristicCount; i++) {
		if (_characteristics[i]->_uuid == uuid) {
			return *_characteristics[i];
		}
	}
	return BleCharacteristic();
}