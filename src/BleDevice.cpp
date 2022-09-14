#include <BleDevice.h>
#include <Arduino.h>

BleDevice::BleDevice(BleScan scan, MacAddress address, rssi_t rssi) {
	memcpy(_scanData, scan._scanData, scan._scanSize);
	_scan                 = BleScan(_scanData, scan._scanSize);
	_address              = address;
	_rssi                 = rssi;
	_flags.flags.initialized = true;
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
	// todo: implement
	// this will be a blocking function
	return false;
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