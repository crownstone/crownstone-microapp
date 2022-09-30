#include <BleService.h>

// Only used for local services
BleService::BleService(const char* uuid) {
	_uuid                    = Uuid(uuid);
	if (!_uuid.valid()) {
		// If uuid not valid, return early
		return;
	}
	_flags.flags.remote      = false;
	_flags.flags.initialized = true;
}

// Only used for remote (discovered) services
BleService::BleService(microapp_sdk_ble_uuid_t* uuid) {
	_uuid = Uuid(uuid->uuid, uuid->type);
	_flags.flags.remote      = true;
	_flags.flags.initialized = true;
}

BleService::operator bool() const {
	return _flags.flags.initialized;
}

// Only for local services
microapp_sdk_result_t BleService::addLocalService() {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	microapp_sdk_result_t result;
	// If uuid not registered, register it first
	// The registered check is not strictly necessary
	// since it's checked internally in registerCustom as well
	// but it's intuitive so I left it
	if (!_uuid.registered()) {
		result = _uuid.registerCustom();
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}
	// Add self via call to bluenet
	uint8_t* payload                                   = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest                     = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType                     = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                             = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                                   = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type                        = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_ADD_SERVICE;
	bleRequest->peripheral.connectionHandle            = 0;
	bleRequest->peripheral.requestAddService.uuid.type = _uuid.getType();
	bleRequest->peripheral.requestAddService.uuid.uuid = _uuid.uuid16();

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	_handle = bleRequest->peripheral.handle;

	// add characteristics
	for (uint8_t i = 0; i < _characteristicCount; i++) {
		result = _characteristics[i]->addLocalCharacteristic(_handle);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}
	_flags.flags.added = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleService::getCharacteristic(uint16_t handle, BleCharacteristic** characteristic) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	for (uint8_t i = 0; i < _characteristicCount; i++) {
		if (_characteristics[i]->_valueHandle == handle) {
			*characteristic = _characteristics[i];
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		if (_characteristics[i]->_cccdHandle == handle) {
			*characteristic = _characteristics[i];
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

// Only for remote services
microapp_sdk_result_t BleService::addDiscoveredCharacteristic(BleCharacteristic* characteristic) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (!_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	if (_characteristicCount >= MAX_CHARACTERISTICS) {
		return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
	}
	_characteristics[_characteristicCount] = characteristic;
	_characteristicCount++;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

String BleService::uuid() {
	if (!_flags.flags.initialized) {
		return String(nullptr);
	}
	return String(_uuid.string());
}

// Only for local services
void BleService::addCharacteristic(BleCharacteristic& characteristic) {
	if (_characteristicCount >= MAX_CHARACTERISTICS) {
		return;
	}
	if (_flags.flags.remote) {
		return;
	}
	_characteristics[_characteristicCount] = &characteristic;
	_characteristicCount++;
}

uint8_t BleService::characteristicCount() {
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

BleCharacteristic& BleService::characteristic(const char* uuidString) {
	Uuid uuid(uuidString);
	for (int i = 0; i < _characteristicCount; i++) {
		if (_characteristics[i]->_uuid == uuid) {
			return *_characteristics[i];
		}
	}
	static BleCharacteristic empty;
	return empty;
}