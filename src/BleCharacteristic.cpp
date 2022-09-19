#include <BleCharacteristic.h>

BleCharacteristic::BleCharacteristic(
		const char* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize, bool remote) {
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
	_flags.flags.remote      = remote;
	_flags.flags.initialized = true;
}

BleCharacteristic::BleCharacteristic(microapp_sdk_ble_uuid_t* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize, bool remote) {
	_uuid = Uuid(uuid->uuid);
	if (uuid->type != CS_MICROAPP_SDK_BLE_UUID_STANDARD) {
		_uuid.setCustomId(uuid->type);
	}
	_properties              = properties;
	_value                   = value;
	_valueSize               = valueSize;
	_flags.flags.remote      = remote;
	_flags.flags.initialized = true;
}

// Outside the Ble/BleService class, only this constructor (and the empty) can be called
// remote = false means the characteristic is local (crownstone = peripheral)
BleCharacteristic::BleCharacteristic(const char* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize)
		: BleCharacteristic(uuid, properties, value, valueSize, false) {}

microapp_sdk_result_t BleCharacteristic::add(uint16_t serviceHandle) {
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

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return result;
	}
	_handle            = bleRequest->peripheral.handle;
	_flags.flags.added = true;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

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

bool BleCharacteristic::writeValueLocal(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (_flags.flags.remote) {
		return false;
	}
	if (length > _valueSize) {
		length = _valueSize;
	}
	// if buffer points to different location,
	// copy to original location since that can't be changed
	if (_value != buffer) {
		memcpy(_value, buffer, length);
	}
	_valueLength = length;

	// notify bluenet with VALUE_SET
	uint8_t* payload                            = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest              = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType              = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                      = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                            = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type                 = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_VALUE_SET;
	bleRequest->peripheral.handle               = _handle;
	bleRequest->peripheral.requestValueSet.size = _valueLength;
	// todo: set connectionHandle?

	sendMessage();

	// todo: notify if subscribed
	return (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
}

bool BleCharacteristic::writeValueRemote(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (!_flags.flags.remote) {
		return false;
	}
	// todo: implement
	return false;
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

int BleCharacteristic::valueSize() {
	return _valueSize;
}

uint8_t* BleCharacteristic::value() {
	if (!_flags.flags.initialized) {
		return nullptr;
	}
	return _value;
}

int BleCharacteristic::valueLength() {
	return _valueLength;
}

bool BleCharacteristic::writeValue(uint8_t* buffer, uint16_t length) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (_flags.flags.remote) {
		return writeValueRemote(buffer, length);
	}
	else {
		return writeValueLocal(buffer, length);
	}
}

void BleCharacteristic::setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice, BleCharacteristic)) {
	if (!_flags.flags.initialized) {
		return;
	}
	// Only defined for peripheral role
	if (_flags.flags.remote) {
		return;
	}
	// todo: implement
}

bool BleCharacteristic::written() {
	if (!_flags.flags.initialized) {
		return false;
	}
	// Only defined for peripheral role
	if (_flags.flags.remote) {
		return false;
	}
	bool result = _flags.flags.written;
	// Clear the flag upon this call
	_flags.flags.written = false;
	return result;
}

bool BleCharacteristic::subscribed() {
	if (!_flags.flags.initialized) {
		return false;
	}
	// Only defined for peripheral role
	if (_flags.flags.remote) {
		return false;
	}
	return _flags.flags.subscribed;
}
