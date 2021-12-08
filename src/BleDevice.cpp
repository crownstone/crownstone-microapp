#include <BleDevice.h>

microapp_ble_dev_t* BleDevice::rawData() {
	return &_device;
}

String BleDevice::address() {
	if (_flags.flags.cachedAddress) { // if already cached address
		return String(_address,MAC_ADDRESS_STRING_LENGTH);
	}
	MACaddress mac;
	memcpy(mac.byte,_device.addr,MAC_ADDRESS_LENGTH);
	convertMacToString(mac,_address);
	_flags.flags.cachedAddress = true;
	return String(_address,MAC_ADDRESS_STRING_LENGTH);
}

int8_t BleDevice::rssi() {
	return _device.rssi;
}

bool BleDevice::hasLocalName() {
	if (!_flags.flags.checkedLocalName) { // if not yet checked
		data_ptr_t cln;
		data_ptr_t sln;
		_flags.flags.hasCompleteLocalName = findAdvType(GapAdvType::CompleteLocalName,_device.data,_device.dlen,&cln);
		_flags.flags.hasShortenedLocalName = findAdvType(GapAdvType::ShortenedLocalName,_device.data,_device.dlen,&sln);
		_flags.flags.checkedLocalName = true;
	}
	return (_flags.flags.hasCompleteLocalName || _flags.flags.hasShortenedLocalName); // either complete local name or shortened local name
}

String BleDevice::localName() {
	// check if we have a local name
	if (!hasLocalName()) {
		_localNameLen = 0;
		return String(_localName,_localNameLen);
	}
	// check if we have already cached the name
	if (_flags.flags.cachedLocalName) {
		return String(_localName,_localNameLen);
	}
	// if local name field available but not yet cached, get it from _device.data
	data_ptr_t ln;
	if (_flags.flags.hasCompleteLocalName) {
		findAdvType(GapAdvType::CompleteLocalName,_device.data,_device.dlen,&ln);
	}
	else { // hasShortenedLocalName
		findAdvType(GapAdvType::ShortenedLocalName,_device.data,_device.dlen,&ln);
	}
	_localNameLen = ln.len;
	memcpy(_localName,ln.data,_localNameLen);
	_flags.flags.cachedLocalName = true;
	return String(_localName,_localNameLen);
}