#include <BleDevice.h>

BleDevice::BleDevice(const microapp_ble_device_t & dev) {
	_device = dev;
	_address = MacAddress(_device.addr);
	_flags.flags.nonEmpty = true;
}

String BleDevice::address() {
	if (_address.isInitialized()) { // if already cached address
		return _address.getString();
	}
	_address = MacAddress(_device.addr);
	return _address.getString();
}

int8_t BleDevice::rssi() {
	return _device.rssi;
}

bool BleDevice::hasLocalName() {
	if (!_flags.flags.checkedLocalName) { // if not yet checked
		data_ptr_t cln;
		data_ptr_t sln;
		_flags.flags.hasCompleteLocalName = findAdvertisementDataType(GapAdvType::CompleteLocalName,&cln);
		_flags.flags.hasShortenedLocalName = findAdvertisementDataType(GapAdvType::ShortenedLocalName,&sln);
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
		findAdvertisementDataType(GapAdvType::CompleteLocalName,&ln);
	}
	else { // hasShortenedLocalName
		findAdvertisementDataType(GapAdvType::ShortenedLocalName,&ln);
	}
	_localNameLen = ln.len;
	memcpy(_localName,ln.data,_localNameLen);
	_flags.flags.cachedLocalName = true;
	return String(_localName,_localNameLen);
}

bool BleDevice::connect() {
	uint8_t *payload = getOutgoingMessagePayload();
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)(payload);
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_CONNECT;
	memcpy(ble_cmd->addr, _address.getBytes(), MAC_ADDRESS_LENGTH);

//	global_buf_out.length = sizeof(microapp_ble_cmd_t);

	// TODO: sendMessage should return ERR_SUCCESS (0) on success
	//       More importantly, this only communicates the intent of a connection.
	//       A connection is an asynchronous sequence of events not under the control of the microapp.
	//       This has to be captured on the microapp side (in this library).
	//       See e.g. the implementation of delay. There we call sendMessage until we are satisfied
	//       (in this case a simple loop). Here we have to call sendMessage until we are satisfied
	//       with the result.
	int res = (sendMessage() == 0);

	if (res == 0) {
		_flags.flags.connected = true;
	}
	return res;
}

bool BleDevice::connected() {
	return _flags.flags.connected;
}

bool BleDevice::findAdvertisementDataType(GapAdvType type, data_ptr_t* foundData) {
	uint8_t i = 0;
	foundData->data = nullptr;
	foundData->len = 0;
	while (i < _device.dlen-1) {
		uint8_t fieldLen = _device.data[i];
		uint8_t fieldType = _device.data[i+1];
		if (fieldLen == 0 || i + 1 + fieldLen > _device.dlen) {
			return false;
		}
		if (fieldType == type) {
			foundData->data = &_device.data[i+2];
			foundData->len = fieldLen-1;
			return true;
		}
		i += fieldLen+1;
	}
	return false;
}
