#include <ArduinoBLE.h>

// Wrapper for handleScanEvent since class functions can't be registered as a callback
void handleScanEventWrapper(microapp_ble_device_t device) {
	BLE.handleScanEvent(device);
}

// Filters and forwards the bluenet scanned device event interrupt to the user callback
void Ble::handleScanEvent(microapp_ble_device_t device) {
	_bleDevice = BleDevice(device);
	if (!filterScanEvent(_bleDevice)) {
		return; // advertisement does not match filter, so do not call user callback
	}
	// now call the user registered callback
	void (*callback_func)(BleDevice) = (void (*)(BleDevice)) _scannedDeviceCallback;
	callback_func(_bleDevice);
}

bool Ble::filterScanEvent(BleDevice device) {
	switch (_activeFilter.type) {
		case BleFilterAddress: {
			// Serial.print("Scanned device MAC address "); Serial.println(device.address().c_str());
			if (memcmp(device.rawData()->addr,_activeFilter.address.byte,MAC_ADDRESS_LENGTH) != 0) { // MAC address does not match filter
				return false;
			}
			break;
		}
		case BleFilterLocalName: {
			if (!device.hasLocalName()) { // no advertised local name
				return false;
			}
			String deviceName = device.localName();
			if (deviceName.length() != _activeFilter.len) { // device name and filter name don't have same length
				return false;
			}
			if (memcmp(deviceName.c_str(),_activeFilter.name,_activeFilter.len) != 0) { // local name doesn't match filter name
				return false;
			}
			break;
		}
		case BleFilterUuid: {
			// TODO: refactor. Now filters ads of type service data with as first element the filtered uuid,
			// which is not the same as what the official ArduinoBLE library does
			microapp_ble_device_t* rawDevice = device.rawData();
			data_ptr_t serviceData;
			if (!findAdvType(GapAdvType::ServiceData, rawDevice->data, rawDevice->dlen,&serviceData)) { // no service data in advertisement
				return false;
			}
			uint16_t uuid = ((serviceData.data[1] << 8) | serviceData.data[0]);
			if (uuid != _activeFilter.uuid) { // service data uuid does not match filter uuid
				return false;
			}
			break;
		}
		case BleFilterNone:
		default:
			break;
	}
	return true;
}

void Ble::setEventHandler(BleEventType type, void (*callback)(BleDevice)) {
	Serial.println("Setting event handler");
	// TODO: do something with type. For now assume type is BleEventDeviceScanned
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;
	// Registered callback in bluenet is actually handleScanEventWrapper
	ble_cmd->callback = (uintptr_t)(handleScanEventWrapper);

	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);

	// Now register the user callback within the Ble object
	_scannedDeviceCallback = (uintptr_t)(callback);
}

bool Ble::scan(bool withDuplicates) {
	if (_isScanning) { // already scanning
		return true;
	}

	Serial.println("Starting BLE device scanning");

	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_START;
	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);
	// TODO: check for return message from bluenet
	_isScanning = true;

	return true;
}

bool Ble::scanForName(const char* name, bool withDuplicates) {
	_activeFilter.type = BleFilterLocalName;
	_activeFilter.len = strlen(name);
	memcpy(_activeFilter.name, name, _activeFilter.len);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForAddress(const char* address, bool withDuplicates) {
	_activeFilter.type = BleFilterAddress;
	_activeFilter.address = convertStringToMac(address);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForUuid(const char* uuid, bool withDuplicates) {
	_activeFilter.type = BleFilterUuid;
	_activeFilter.uuid = convertStringToUuid(uuid);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

void Ble::stopScan() {
	if (!_isScanning) { // already not scanning
		return;
	}

	Serial.println("Stopping BLE device scanning");

	_activeFilter.type = BleFilterNone; // reset filter

	// send a message to bluenet commanding it to stop forwarding ads to microapp
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_STOP;
	global_msg.length = sizeof(microapp_ble_cmd_t);

	_isScanning = false;
	// TODO: check for return message from bluenet
	sendMessage(&global_msg);
}

BleFilter* Ble::getFilter() {
	return &_activeFilter;
}
