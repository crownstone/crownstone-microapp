#include <BleDevice.h>
#include <Serial.h>

microapp_ble_dev_t* BleDevice::rawData() {
	return &_device;
}

String BleDevice::address() {
	MACaddress mac;
	memcpy(mac.byte,_device.addr,MAC_ADDRESS_LENGTH);
	convertMacToString(mac,_address);
	return _address;
}

int8_t BleDevice::rssi() {
	return _device.rssi;
}

bool BleDevice::hasLocalName() {
	data_ptr_t cln;
	data_ptr_t sln;
	_hasCompleteLocalName = findAdvType(GapAdvType::CompleteLocalName,_device.data,_device.dlen,&cln);
	_hasShortenedLocalName = findAdvType(GapAdvType::ShortenedLocalName,_device.data,_device.dlen,&sln);
	return (_hasCompleteLocalName || _hasShortenedLocalName); // either complete local name or shortened local name
}

String BleDevice::localName() {
	// first check if we have a local name
	if (!hasLocalName()) {
		_localName[0] = 0;
		return _localName;
	}
	data_ptr_t ln;
	if (_hasCompleteLocalName) {
		findAdvType(GapAdvType::CompleteLocalName,_device.data,_device.dlen,&ln);
	}
	else { // _hasShortenedLocalName
		findAdvType(GapAdvType::ShortenedLocalName,_device.data,_device.dlen,&ln);
	}
	memcpy(_localName,ln.data,ln.len);
	_localName[ln.len] = 0; // set escape char
	return _localName;
}