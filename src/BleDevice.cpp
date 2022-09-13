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
	ble_ad_t localName = _scan.localName();
	return (localName.len != 0);
}

String BleDevice::localName() {
	// todo: implement
	// copy localName to a local buffer with termination char and return
	// for now this is deemed to cost too much memory with little gain
	return String(nullptr);
}

bool BleDevice::connect() {
	// todo: implement
	// this will be a blocking function
	return false;
}

bool BleDevice::findAdvertisementDataType(GapAdvType type, ble_ad_t* foundData) {
	return _scan.findAdvertisementDataType(type, foundData);
}