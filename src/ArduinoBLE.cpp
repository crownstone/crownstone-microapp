#include <ArduinoBLE.h>


Ble::Ble() {
	for (int i = 0; i < MAX_CALLBACKS; i++) {
		callbackContext[i].callback = nullptr;
		callbackContext[i].filled = false;
	}
}

// Wrapper for handleScanEvent since class functions can't be registered as a callback
//void handleScanEventWrapper(microapp_ble_device_t device) {
//	BLE.handleScanEvent(device);
//}

// Filters and forwards the bluenet scanned device event interrupt to the user callback
/*
void Ble::handleScanEvent(microapp_ble_device_t device) {
	BleDevice newDevice = BleDevice(device);
	if (!filterScanEvent(newDevice)) {
		// advertisement does not match filter, so do not call user callback
		return;
	}
	// TODO: find a way to preserve cached info like device name if new ad is from same device as active device
	_activeDevice = newDevice;
	// now call the user registered callback
	void (*callback_func)(BleDevice) = (void (*)(BleDevice)) _scannedDeviceCallback;
	callback_func(newDevice);
}*/

bool Ble::filterScanEvent(BleDevice device) {
	switch (_activeFilter.type) {
		case BleFilterAddress: {
			// Serial.print("Scanned device MAC address "); Serial.println(device.address().c_str());
			// Serial.print("Filter MAC address "); Serial.println(_activeFilter.address.getString().c_str());
			if (device._address != _activeFilter.address) { // MAC address does not match filter
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
			data_ptr_t serviceData;
			if (!device.findAdvertisementDataType(GapAdvType::ServiceData,&serviceData)) { // no service data in advertisement
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

/*
void BLEmapping(microapp_ble_device_t *deviceIn, BleDevice *deviceOut) {
	deviceOut->rssi = deviceIn->rssi;
}*/

/*
 * In the code for ArduinoBLE there are setEventHandler's for events like BLEConnected and BLEDisconnected. These are
 * formatted like:
 *
 * ```
 *     BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
 *     void blePeripheralConnectHandler(BLEDevice central) { }
 * ```
 *
 * These events do not happen very often, hence this might be fine. However, in our case we want to get device info
 * for every scan request. This means we should not pass by value such a large object as BLEDevice. The smallest change
 * for the user is to use pass by reference instead:
 * ```
 *     BLE.setEventHandler(BLEDeviceScanned, bleDeviceScannedHandler);
 *     void bleDeviceScannedHandler(BLEDevice & central) { }
 * ```
 * Apart from this each advertisement can in theory passed through the bluenet code towards the microapp, but not in
 * practice. This is because handling the advertisement in the handler takes time. Especially if the microapp user
 * logs for example all kind of info like RSSI values, MAC address, etc. This means that we need to throttle.
 * The smartest way to throttle is by limiting the number of callbacks coming back to bluenet from the microapp within
 * a single tick. Hence, if the microapp implements a very efficient handler without roundtrips towards bluenet it can
 * receive many advertisements.
 *
 * In either way, we can't just immediately set the event handler and evoke it later. We need one level of indirection.
 * We write a struct in which the event handler is written, then we register our own event handler which will call the
 * user-facing one. After the user-facing handler is done it falls back to our library event handler upon which we can
 * send a SOFT_INTERRUPT_END message. Alternatively, we can keep this information local and update the  lock-state
 * locally as well.
 *
 * Mmm... Perhaps just keep it this way. Yes, the callback handler gets a copy of the object, but that means that it
 * is completely reentrant.
 *
 * References:
 * - https://www.arduino.cc/en/Reference/ArduinoBLEBLEsetEventHandler
 */

/*
 * An ordinary C function to keep the callback simple.
 */
void bleCallback(void *args, void* buf) {
	microapp_ble_device_t *dev = (microapp_ble_device_t*)buf;
	bleCallbackContext *context = (bleCallbackContext*)args;

	if (!context->callback) {
		return;
	}

	// Create temporary object on the stack
	BleDevice bleDevice = BleDevice(*dev);
	// Call the callback with a copy of this object
	context->callback(bleDevice);
	
	microapp_cmd_t *cmd = (microapp_cmd_t*)&global_msg;
	cmd->cmd = CS_MICROAPP_COMMAND_CALLBACK_END;
	cmd->id = context->id;
	sendMessage(&global_msg);
}

/*
 * Set callback, but not directly... We register a wrapper function that calls this callback so we have the possibility
 * to send a "callback end" command after the callback has been executed.
 */
void Ble::setEventHandler(BleEventType type, void (*callback)(BleDevice)) {

	int callbackContextId = -1;
	for (int i = 0; i < MAX_CALLBACKS; ++i) {
		if (callbackContext[i].filled == false) {
			callbackContextId = i;
			break;
		}
	}
	if (callbackContextId < 0) {
		Serial.println("No space for new event handler");
		return;
	}
	bleCallbackContext & context = callbackContext[callbackContextId];

	Serial.println("Setting event handler");
	// TODO: do something with type. For now assume type is BleEventDeviceScanned
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	//Serial.print("Write command to address: ");
	//Serial.println((unsigned int)ble_cmd);
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;

	// Set identifier now to 0 (assuming a single callback)
	ble_cmd->id = 0;

	global_msg.length = sizeof(microapp_ble_cmd_t);

	context.callback = callback;
	context.filled = true;
	context.id = ble_cmd->id;

	callback_t cb;
	cb.id = ble_cmd->id;
	cb.type = CALLBACK_TYPE_BLE;
	cb.callback = bleCallback;
	cb.arg = &callbackContext[callbackContextId];
//	cb.callback = reinterpret_cast<callbackFunction>(callback);
	//cb.arg = _device[ble_cmd->id]; // if there are more than 1 device
	//cb.arg = (void*)&_activeDevice;
	registerCallback(&cb);
	
	sendMessage(&global_msg);

	// Now register the user callback within the Ble object
	//_scannedDeviceCallback = (uintptr_t)(callback);
}

bool Ble::scan(bool withDuplicates) {
	if (_isScanning) { // already scanning
		return true;
	}

	//Serial.println("Starting BLE device scanning");

	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->header.ack = false;
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_START;
	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);

	bool success = ble_cmd->header.ack;
	if (!success) {
		return success;
	}
	_isScanning = true;
	return success;
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
	_activeFilter.address = MacAddress(address);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForUuid(const char* uuid, bool withDuplicates) {
	_activeFilter.type = BleFilterUuid;
	_activeFilter.uuid = convertStringToUuid(uuid);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::stopScan() {
	if (!_isScanning) { // already not scanning
		return true;
	}

	//Serial.println("Stopping BLE device scanning");

	_activeFilter.type = BleFilterNone; // reset filter

	// send a message to bluenet commanding it to stop forwarding ads to microapp
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->header.ack = false;
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_STOP;
	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);
	
	bool success = ble_cmd->header.ack;
	if (!success) {
		return success;
	}
	_isScanning = false;
	return success;
}

BleDevice Ble::available() {
	return _activeDevice;
}

BleFilter* Ble::getFilter() {
	return &_activeFilter;
}
