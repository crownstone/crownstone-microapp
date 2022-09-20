#include <Arduino.h>
#include <BleDevice.h>

BleDevice::BleDevice(BleScan scan, MacAddress address, rssi_t rssi) {
	memcpy(_scanData, scan._scanData, scan._scanSize);
	_scan                     = BleScan(_scanData, scan._scanSize);
	_address                  = address;
	_rssi                     = rssi;
	_flags.flags.isPeripheral = true;
	_flags.flags.initialized  = true;
}

BleDevice::BleDevice(MacAddress address) {
	_address                 = address;
	_flags.flags.isCentral   = true;
	_flags.flags.initialized = true;
}

void BleDevice::onConnect(const uint8_t* address) {
	_flags.flags.connected = true;
}

void BleDevice::onDisconnect() {
	_flags.flags.connected = false;
}

microapp_sdk_result_t BleDevice::addDiscoveredService(BleService* service) {
	_services[_serviceCount] = service;
	_serviceCount++;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t BleDevice::addDiscoveredCharacteristic(BleCharacteristic* characteristic, Uuid& serviceUuid) {
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->_uuid == serviceUuid) {
			return _services[i]->addDiscoveredCharacteristic(characteristic);
		}
	}
	// todo: add service first, then add characteristic
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

microapp_sdk_result_t BleDevice::getCharacteristic(uint16_t handle, BleCharacteristic& characteristic) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (!_flags.flags.isCentral) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		microapp_sdk_result_t result = _services[i]->getCharacteristic(handle, characteristic);
		if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

void BleDevice::poll(int timeout) {
	if (timeout == 0) {
		return;
	}
	delay(timeout);
}

bool BleDevice::connected() {
	return _flags.flags.connected;
}

bool BleDevice::disconnect() {
	// this is a blocking function
	if (!_flags.flags.connected) {
		return false;
	}
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	if (_flags.flags.isCentral) {
		bleRequest->type         = CS_MICROAPP_SDK_BLE_CENTRAL;
		bleRequest->central.type = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_DISCONNECT;
	}
	else if (_flags.flags.isPeripheral) {
		bleRequest->type            = CS_MICROAPP_SDK_BLE_PERIPHERAL;
		bleRequest->peripheral.type = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_DISCONNECT;
	}
	sendMessage();
	if (bleRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}
	// todo: connectionhandle
	uint8_t tries = 5;
	while (_flags.flags.connected) {
		// yield. Upon a disconnect event flag will be cleared
		delay(100);
		if (--tries < 0) {
			return false;
		}
	}
	return true;
}

String BleDevice::address() {
	return String(_address.string());
}

int8_t BleDevice::rssi() {
	return _rssi;
}

bool BleDevice::hasLocalName() {
	return (_scan.localName().len != 0);
}

String BleDevice::localName() {
	ble_ad_t localName = _scan.localName();
	if (localName.len == 0) {
		return String(nullptr);
	}
	static char localNameString[MAX_BLE_ADV_DATA_LENGTH + 1];
	memcpy(localNameString, localName.data, localName.len);
	localNameString[localName.len] = 0;
	return String(localNameString);
}

bool BleDevice::connect() {
	// Only defined for peripheral role
	// todo: implement
	// this will be a blocking function
	return false;
}

bool BleDevice::findAdvertisementDataType(GapAdvType type, ble_ad_t* foundData) {
	return _scan.findAdvertisementDataType(type, foundData);
}