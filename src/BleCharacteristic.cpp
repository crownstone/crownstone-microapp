#include <Arduino.h>
#include <BleCharacteristic.h>

// Only used for local characteristics
BleCharacteristic::BleCharacteristic(const char* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize) {
	if (valueSize > MAX_CHARACTERISTIC_VALUE_SIZE) {
		// silently truncate
		valueSize = MAX_CHARACTERISTIC_VALUE_SIZE;
	}
	_uuid                    = Uuid(uuid);
	if (!_uuid.valid()) {
		// If uuid not valid, return early
		return;
	}
	_properties              = properties;
	_value                   = value;
	_valueSize               = valueSize;
	_valueLength             = valueSize;
	_flags.flags.remote      = false;
	_flags.flags.initialized = true;
}

// Only used for remote characteristics
BleCharacteristic::BleCharacteristic(microapp_sdk_ble_uuid_t* uuid, uint8_t properties) {
	_uuid                    = Uuid(uuid->uuid, uuid->type);
	_properties              = properties;
	_value                   = nullptr;
	_valueSize               = 0;
	_flags.flags.remote      = true;
	_flags.flags.initialized = true;
}

BleCharacteristic::operator bool() const {
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

	// add self
	microapp_sdk_ble_characteristic_options_t options;
	options.read            = _properties & BleCharacteristicProperties::BLERead;
	options.writeNoResponse = _properties & BleCharacteristicProperties::BLEWriteWithoutResponse;
	options.write           = _properties & BleCharacteristicProperties::BLEWrite;
	options.notify          = _properties & BleCharacteristicProperties::BLENotify;
	options.indicate        = _properties & BleCharacteristicProperties::BLEIndicate;
	options.autoNotify      = true;

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
	_valueHandle            = bleRequest->peripheral.handle;
	_flags.flags.added = true;
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
	bleRequest->peripheral.handle               = _valueHandle;
	bleRequest->peripheral.requestValueSet.size = _valueLength;
	bleRequest->peripheral.connectionHandle     = 0;

	sendMessage();

	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
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
	bleRequest->central.requestWrite.handle      = _valueHandle;
	bleRequest->central.connectionHandle         = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return result;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return result;
	}
	// Block until write event happens
	uint8_t tries = timeout / MICROAPP_LOOP_INTERVAL_MS;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(MICROAPP_LOOP_INTERVAL_MS);
		if (--tries == 0) {
			return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT;
		}
	}
	// Clear flag
	_flags.flags.writtenToRemote = false;
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
	bleRequest->central.requestRead.valueHandle = _valueHandle;
	bleRequest->central.connectionHandle        = 0;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return result;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return result;
	}
	// Block until read event happens
	uint8_t tries = timeout / MICROAPP_LOOP_INTERVAL_MS;
	while (!_flags.flags.remoteValueRead) {
		// yield. Upon a read event flag will be set
		delay(MICROAPP_LOOP_INTERVAL_MS);
		if (--tries == 0) {
			return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT;
		}
	}
	// Clear flag
	_flags.flags.remoteValueRead = false;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onRemoteWritten() {
	_flags.flags.writtenToRemote = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onRemoteRead(microapp_sdk_ble_central_event_read_t* eventRead) {
	// Data size is limited by valueSize of characteristic
	uint8_t size = eventRead->size;
	if (size > _valueSize) {
		size = _valueSize;
	}
	// Copy data to value pointer
	memcpy(_value, eventRead->data, size);
	_valueLength = size;
	// Set flag so that blocking readValue function can return
	_flags.flags.remoteValueRead = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onRemoteNotification(microapp_sdk_ble_central_event_notification_t* eventNotification) {
	// Data size is limited by valueSize of characteristic
	uint8_t size = eventNotification->size;
	if (size > _valueSize) {
		size = _valueSize;
	}
	// Do not copy data. That can be done using readValue,
	// where the user provides a buffer to copy the data to.
	// Only set new value length so user may request new length
	_valueLength = size;
	// Set flag so user may poll whether notify happened
	_flags.flags.remoteValueUpdated = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onLocalWritten(microapp_sdk_ble_peripheral_event_write_t* eventWrite) {
	_valueLength = eventWrite->size;
	_flags.flags.writtenAsLocal = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onLocalSubscribed() {
	_flags.flags.subscribed = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onLocalUnsubscribed() {
	_flags.flags.subscribed = false;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleCharacteristic::onLocalNotificationDone() {
	_flags.flags.localNotificationDone = true;
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
	// Write to _cccdValue: bit 0 for notify, bit 1 for indicate
	// We only use the first two bits, but BLE specs says the value should be 2 bytes
	// If notify is available, we use that. Otherwise, we try indicate
	if (_properties & BleCharacteristicProperties::BLENotify) {
		_cccdValue = 1;
	}
	else if (_properties & BleCharacteristicProperties::BLEIndicate) {
		_cccdValue = 1 << 1;
	}
	else {
		// Both are not allowed. Subscribing not possible
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
	bleRequest->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_WRITE;
	bleRequest->central.requestWrite.handle = _cccdHandle;
	bleRequest->central.requestWrite.buffer = reinterpret_cast<uint8_t*>(&_cccdValue);
	bleRequest->central.requestWrite.size = 2;
	bleRequest->central.connectionHandle = BLE_CONNECTION_HANDLE_PLACEHOLDER;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
		result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return true;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return false;
	}
	// Block until write event happens
	uint8_t tries = timeout / MICROAPP_LOOP_INTERVAL_MS;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(MICROAPP_LOOP_INTERVAL_MS);
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
	if (!canUnsubscribe()) {
		return false;
	}
	// Clear the notify and indicate bits both
	_cccdValue = 0;
	// Clear flag
	_flags.flags.writtenToRemote = false;

	microapp_sdk_result_t result;
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest       = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType       = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack               = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                     = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_WRITE;
	bleRequest->central.connectionHandle = BLE_CONNECTION_HANDLE_PLACEHOLDER;
	bleRequest->central.requestWrite.handle = _cccdHandle;
	bleRequest->central.requestWrite.buffer = reinterpret_cast<uint8_t*>(&_cccdValue);
	bleRequest->central.requestWrite.size = 2;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return true;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return false;
	}
	// Block until write event happens
	uint8_t tries = timeout / MICROAPP_LOOP_INTERVAL_MS;
	while (!_flags.flags.writtenToRemote) {
		// yield. Upon a write event flag will be set
		delay(MICROAPP_LOOP_INTERVAL_MS);
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
