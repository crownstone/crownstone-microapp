#include <BleDevice.h>

BleDevice::BleDevice(microapp_sdk_ble_scan_event_t* scanEvent) {
	_scanEvent            = scanEvent;
	_address              = MacAddress(_scanEvent->address.address);
	_flags.flags.nonEmpty = true;
}

String BleDevice::address() {
	return String(_address.string());
}

int8_t BleDevice::rssi() {
	return _scanEvent->rssi;
}

bool BleDevice::hasLocalName() {
	if (!_flags.flags.checkedLocalName) {  // if not yet checked
		data_ptr_t cln;
		data_ptr_t sln;
		_flags.flags.hasCompleteLocalName  = findAdvertisementDataType(GapAdvType::CompleteLocalName, &cln);
		_flags.flags.hasShortenedLocalName = findAdvertisementDataType(GapAdvType::ShortenedLocalName, &sln);
		_flags.flags.checkedLocalName      = true;
	}
	return (_flags.flags.hasCompleteLocalName
			|| _flags.flags.hasShortenedLocalName);  // either complete local name or shortened local name
}

String BleDevice::localName() {
	// check if we have a local name
	if (!hasLocalName()) {
		_localNameLen = 0;
		return String(_localName, _localNameLen);
	}
	// check if we have already cached the name
	if (_flags.flags.cachedLocalName) {
		return String(_localName, _localNameLen);
	}
	// if local name field available but not yet cached, get it from _device.data
	data_ptr_t ln;
	if (_flags.flags.hasCompleteLocalName) {
		findAdvertisementDataType(GapAdvType::CompleteLocalName, &ln);
	}
	else {  // hasShortenedLocalName
		findAdvertisementDataType(GapAdvType::ShortenedLocalName, &ln);
	}
	_localNameLen = ln.len;
	memcpy(_localName, ln.data, _localNameLen);
	_flags.flags.cachedLocalName = true;
	return String(_localName, _localNameLen);
}

bool BleDevice::connect() {
	// TODO: implement
	_flags.flags.connected = false;
	return false;
}

bool BleDevice::connected() {
	return _flags.flags.connected;
}

bool BleDevice::findAdvertisementDataType(GapAdvType type, data_ptr_t* foundData) {
	uint8_t i       = 0;
	foundData->data = nullptr;
	foundData->len  = 0;
	while (i < _scanEvent->size - 1) {
		uint8_t fieldLen  = _scanEvent->data[i];
		uint8_t fieldType = _scanEvent->data[i + 1];
		if (fieldLen == 0 || i + 1 + fieldLen > _scanEvent->size) {
			return false;
		}
		if (fieldType == type) {
			foundData->data = &_scanEvent->data[i + 2];
			foundData->len  = fieldLen - 1;
			return true;
		}
		i += fieldLen + 1;
	}
	return false;
}

bool BleDevice::findServiceDataUuid(uuid16_t uuid) {
	GapAdvType serviceUuidListTypes[2] = {
			GapAdvType::IncompleteList16BitServiceUuids, GapAdvType::CompleteList16BitServiceUuids};
	data_ptr_t adData;
	for (int i = 0; i < 2; i++) {
		if (findAdvertisementDataType(serviceUuidListTypes[i], &adData)) {
			// check adData for uuid
			int j = 0;
			while (j < adData.len) {
				if (uuid == ((adData.data[j + 1] << 8) | adData.data[j])) {
					return true;
				}
				j += 2;  // for 16 bit uuids, shift 2 bytes
			}
		}
	}
	return false;
}