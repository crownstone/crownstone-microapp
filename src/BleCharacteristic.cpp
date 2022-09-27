#include <Arduino.h>
#include <BleCharacteristic.h>

// Only used for local characteristics
BleCharacteristic::BleCharacteristic(const char* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize) {
	uint8_t len = strlen(uuid);
	if (len != UUID_16BIT_STRING_LENGTH && len != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	if (valueSize > MAX_CHARACTERISTIC_VALUE_SIZE) {
		// silently truncate
		valueSize = MAX_CHARACTERISTIC_VALUE_SIZE;
	}
	_uuid                    = Uuid(uuid);
	_properties              = properties;
	_value                   = value;
	_valueSize               = valueSize;
	_flags.flags.remote      = false;
	_flags.flags.initialized = true;
}

// Only used for remote characteristics
BleCharacteristic::BleCharacteristic(microapp_sdk_ble_uuid_t* uuid, uint8_t properties) {
	_uuid = Uuid(uuid->uuid);
	if (uuid->type != CS_MICROAPP_SDK_BLE_UUID_STANDARD) {
		_uuid.setCustomId(uuid->type);
	}
	_properties              = properties;
	_value                   = nullptr;
	_valueSize               = 0;
	_flags.flags.remote      = true;
	_flags.flags.initialized = true;
}

explicit BleCharacteristic::operator bool() const {
	return _flags.flags.initialized;
}

// Only defined for local characteristics
microapp_sdk_result_t BleCharacteristic::addLocalCharacteristic(uint16_t serviceHandle) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	microapp_sdk_result_t result;
	// if custom uuid, register that first
	if (_uuid.custom()) {
		result = registerCustomUuid();
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}

	// add self
	microapp_sdk_ble_characteristic_options_t options;
	options.read            = _properties & BleCharacteristicProperties::BLERead;
	options.writeNoResponse = _properties & BleCharacteristicProperties::BLEWriteWithoutResponse;
	options.write           = _properties & BleCharacteristicProperties::BLEWrite;
	options.notify          = _properties & BleCharacteristicProperties::BLENotify;
	options.indicate        = _properties & BleCharacteristicProperties::BLEIndicate;

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type    = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_ADD_CHARACTERISTIC;
	bleRequest->peripheral.requestAddCharacteristic.serviceHandle = serviceHandle;
	bleRequest->peripheral.requestAddCharacteristic.uuid.type     = _uuid.getType();
	bleRequest->peripheral.requestAddCharacteristic.uuid.uuid     = _uuid.uuid16();
	bleRequest->peripheral.requestAddCharacteristic.bufferSize    = _valueSize;
	bleRequest->peripheral.requestAddCharacteristic.buffer        = _value;
	bleRequest->peripheral.requestAddCharacteristic.options       = options;
	bleRequest->peripheral.connectionHandle                       = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	_handle            = bleRequest->peripheral.handle;
	_flags.flags.added = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

// Only defined for local characteristics
microapp_sdk_result_t BleCharacteristic::registerCustomUuid() {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	if (!_uuid.custom()) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_UUID_REGISTER;
	memcpy(bleRequest->requestUuidRegister.customUuid, _uuid.bytes(), UUID_128BIT_BYTE_LENGTH);

	sendMessage();
	microapp_sdk_result_t result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	if (memcmp(&bleRequest->requestUuidRegister.uuid.uuid, _uuid.bytes(), UUID_16BIT_BYTE_LENGTH) != 0) {
		// The returned short uuid is not the same as the original
		// (it should be the same)
		return CS_MICROAPP_SDK_ACK_ERROR;
	}
	_uuid.setCustomId(bleRequest->requestUuidRegister.uuid.type);
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

// Only defined for local characteristics
microapp_sdk_result_t BleCharacteristic::writeValueLocal(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	if (length > _valueSize) {
		length = _valueSize;
	}
	microapp_sdk_result_t result;
	// if buffer points to different location than _value
	// copy to original location _value
	if (_value != buffer) {
		memcpy(_value, buffer, length);
	}
	_valueLength = length;

	// Send VALUE_SET
	uint8_t* payload                            = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest              = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType              = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                      = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                            = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type                 = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_VALUE_SET;
	bleRequest->peripheral.handle               = _handle;
	bleRequest->peripheral.requestValueSet.size = _valueLength;
	bleRequest->peripheral.connectionHandle     = 0;

	sendMessage();

	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	if (_flags.flags.subscribed) {
		result = notify();
	}
	return result;
}

// Only defined for remote characteristics
microapp_sdk_result_t BleCharacteristic::writeValueRemote(uint8_t* buffer, uint16_t length, uint32_t timeout) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (!_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	if (length > MAX_CHARACTERISTIC_VALUE_SIZE) {
		length = MAX_CHARACTERISTIC_VALUE_SIZE;
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;

	microapp_sdk_result_t result;
	uint8_t* payload                             = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest               = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType               = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                       = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                             = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type                     = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_WRITE;
	bleRequest->central.requestWrite.buffer      = buffer;
	bleRequest->central.requestWrite.size        = length;
	bleRequest->central.requestWrite.valueHandle = _handle;
	bleRequest->central.connectionHandle         = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	// Block until write event happens
	uint8_t tries = timeout / 1000;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(1000);
		if (--tries == 0) {
			return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT;
		}
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

// Only defined for local characteristics
microapp_sdk_result_t BleCharacteristic::notify(uint32_t timeout) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (_flags.flags.remote) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	// Clear flag
	_flags.flags.localNotificationDone = false;

	microapp_sdk_result_t result;
	uint8_t* payload                            = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest              = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType              = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                      = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                            = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type                 = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_NOTIFY;
	bleRequest->peripheral.handle               = _handle;
	bleRequest->peripheral.requestNotify.size   = _valueLength;
	bleRequest->peripheral.requestNotify.offset = 0;
	bleRequest->peripheral.connectionHandle     = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	// Block until notification_done event happens
	uint8_t tries = timeout / 1000;
	while (!_flags.flags.localNotificationDone) {
		// yield. Upon a notification_done event flag will be set
		delay(1000);
		if (--tries == 0) {
			return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT;
		}
	}
	// Clear flag
	_flags.flags.localNotificationDone = false;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

// Only defined for remote characteristics
microapp_sdk_result_t BleCharacteristic::readValueRemote(uint8_t* buffer, uint16_t length, uint32_t timeout) {
	if (length > MAX_CHARACTERISTIC_VALUE_SIZE) {
		length = MAX_CHARACTERISTIC_VALUE_SIZE;
	}
	// Set value pointer and max size
	_value     = buffer;
	_valueSize = length;

	// Clear flag
	_flags.flags.remoteValueRead = false;

	microapp_sdk_result_t result;
	uint8_t* payload                            = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest              = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType              = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                      = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                            = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type                    = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_READ;
	bleRequest->central.requestRead.valueHandle = _handle;
	bleRequest->central.connectionHandle        = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	// Block until read event happens
	uint8_t tries = timeout / 1000;
	while (!_flags.flags.remoteValueRead) {
		// yield. Upon a read event flag will be set
		delay(1000);
		if (--tries == 0) {
			return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT;
		}
	}
	// Clear flag
	_flags.flags.remoteValueRead = false;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

String BleCharacteristic::uuid() {
	if (!_flags.flags.initialized) {
		return String(nullptr);
	}
	return String(_uuid.string());
}

uint8_t BleCharacteristic::properties() {
	return _properties;
}

uint16_t BleCharacteristic::valueSize() {
	return _valueSize;
}

uint8_t* BleCharacteristic::value() {
	if (!_flags.flags.initialized) {
		return nullptr;
	}
	return _value;
}

uint16_t BleCharacteristic::valueLength() {
	return _valueLength;
}

uint16_t BleCharacteristic::readValue(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return 0;
	}
	if (!_flags.flags.remote) {
		if (_valueLength < length) {
			length = _valueLength;
		}
		memcpy(buffer, _value, length);
		return length;
	}
	else {
		microapp_sdk_result_t result = readValueRemote(buffer, length);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return 0;
		}
		// Clear valueUpdated flag after set on notify
		_flags.flags.remoteValueUpdated = false;
		return _valueLength;
	}
}

bool BleCharacteristic::writeValue(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (_flags.flags.remote) {
		return (writeValueRemote(buffer, length) == CS_MICROAPP_SDK_ACK_SUCCESS);
	}
	else {
		return (writeValueLocal(buffer, length) == CS_MICROAPP_SDK_ACK_SUCCESS);
	}
}

// Only defined for local characteristics
void BleCharacteristic::setEventHandler(BleEventType eventType, CharacteristicEventHandler eventHandler) {
	if (!_flags.flags.initialized || _flags.flags.remote) {
		return;
	}
	microapp_sdk_result_t result;
	result = registerBleEventHandler(eventType, (BleEventHandler)eventHandler);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return;
	}
	if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL)) {
		result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			removeBleEventHandlerRegistration(eventType);
			return;
		}
	}
}

// Only defined for local characteristics
bool BleCharacteristic::written() {
	if (!_flags.flags.initialized || _flags.flags.remote) {
		return false;
	}
	bool result = _flags.flags.writtenAsLocal;
	// Clear the flag upon this call
	_flags.flags.writtenAsLocal = false;
	return result;
}

bool BleCharacteristic::subscribed() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return _flags.flags.subscribed;
}

bool BleCharacteristic::canRead() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return _properties & BleCharacteristicProperties::BLERead;
}

bool BleCharacteristic::canWrite() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return (_properties & BleCharacteristicProperties::BLEWrite
			|| _properties & BleCharacteristicProperties::BLEWriteWithoutResponse);
}

bool BleCharacteristic::canSubscribe() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return (_properties & BleCharacteristicProperties::BLEIndicate
			|| _properties & BleCharacteristicProperties::BLENotify);
}

// Only defined for remote characteristics
bool BleCharacteristic::subscribe(uint32_t timeout) {
	if (!_flags.flags.initialized || !_flags.flags.remote) {
		return false;
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;

	microapp_sdk_result_t result;
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest       = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType       = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack               = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                     = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_SUBSCRIBE;
	bleRequest->central.connectionHandle = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}
	// Block until write event happens
	uint8_t tries = timeout / 1000;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(1000);
		if (--tries == 0) {
			return false;
		}
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;
	return true;
}

bool BleCharacteristic::canUnsubscribe() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return (_properties & BleCharacteristicProperties::BLEIndicate
			|| _properties & BleCharacteristicProperties::BLENotify);
}

// Only defined for remote characteristics
bool BleCharacteristic::unsubscribe(uint32_t timeout) {
	if (!_flags.flags.initialized || !_flags.flags.remote) {
		return false;
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;

	microapp_sdk_result_t result;
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest       = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType       = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack               = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                     = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_UNSUBSCRIBE;
	bleRequest->central.connectionHandle = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}
	// Block until write event happens
	uint8_t tries = timeout / 1000;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(1000);
		if (--tries == 0) {
			return false;
		}
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;
	return true;
}

// Only defined for remote characteristics
bool BleCharacteristic::valueUpdated() {
	if (!_flags.flags.initialized || !_flags.flags.remote) {
		return false;
	}
	return _flags.flags.remoteValueUpdated;
}
