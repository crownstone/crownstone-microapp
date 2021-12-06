#include <BleDevice.h>

microapp_ble_dev_t* BleDevice::rawData() {
	return _dev;
}

String BleDevice::address() {
	MACaddress mac;
	memcpy(mac.byte,_dev->addr,MAC_ADDRESS_LENGTH);
	convertMacToString(mac,_address);
	return _address;
}

int8_t BleDevice::rssi() {
	return _dev->rssi;
}

bool BleDevice::hasLocalName() {
	data_ptr_t cln;
	data_ptr_t sln;
	_hasCLN = findAdvType(GapAdvType::CompleteLocalName,_dev->data,_dev->dlen,&cln);
	_hasSLN = findAdvType(GapAdvType::ShortenedLocalName,_dev->data,_dev->dlen,&sln);
	return (_hasCLN || _hasSLN); // either complete local name or shortened local name
}

String BleDevice::localName() {
	// first check if we have a local name
	if (!hasLocalName()) {
		_localName[0] = 0;
		return _localName;
	}
	data_ptr_t ln;
	if (_hasCLN) {
		findAdvType(GapAdvType::CompleteLocalName,_dev->data,_dev->dlen,&ln);
	}
	else { // _hasSLN
		findAdvType(GapAdvType::ShortenedLocalName,_dev->data,_dev->dlen,&ln);
	}
	memcpy(_localName,ln.data,ln.len);
	return _localName;
}