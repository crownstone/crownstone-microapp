#include <ArduinoBLE.h>

/*
 * An ordinary C function. Calls internal handler
 */
microapp_sdk_result_t handleBleInterrupt(void* buf) {
	if (buf == nullptr) {
		return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
	}
	microapp_sdk_ble_t* bleInterrupt = reinterpret_cast<microapp_sdk_ble_t*>(buf);
	return BLE.handleInterrupt(bleInterrupt);
}

/*
 * Internal interrupt handler.
 * Retrieves context, checks type of interrupt and takes appropriate action
 */
microapp_sdk_result_t Ble::handleInterrupt(microapp_sdk_ble_t* bleInterrupt) {
	// First get interrupt context
	int interruptContextId = -1;
	for (int i = 0; i < MAX_BLE_INTERRUPT_REGISTRATIONS; ++i) {
		if (_bleInterruptContext[i].filled == false) {
			continue;
		}
		if (_bleInterruptContext[i].type == bleInterrupt->type) {
			interruptContextId = i;
			break;
		}
	}
	if (interruptContextId < 0) {
		// Interrupt context not found
		return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
	}
	BleInterruptContext& context = _bleInterruptContext[interruptContextId];

	if (context.eventHandler == nullptr) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}

	// Based on the type of event we will take action
	switch (context.type) {
		case CS_MICROAPP_SDK_BLE_SCAN_SCANNED_DEVICE: {
			// Save to the stack
			// Create temporary object on the stack
			BleDevice bleDevice = BleDevice(bleInterrupt);
			if (filterScanEvent(bleDevice)) {
				// Call the event handler with a copy of this object
				context.eventHandler(bleDevice);
			}
			break;
		}
		case CS_MICROAPP_SDK_BLE_CONNECTION_CONNECTED: {
			// TODO: implement
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
		case CS_MICROAPP_SDK_BLE_CONNECTION_DISCONNECTED: {
			// TODO: implement
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
	}
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

/*
 * Set interrupt handler, but not directly...
 * We register a wrapper function that calls the passed handler
 */
bool Ble::setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice)) {

	// Register the interrupt context locally
	int interruptContextId = -1;
	for (int i = 0; i < MAX_BLE_INTERRUPT_REGISTRATIONS; ++i) {
		if (_bleInterruptContext[i].filled == false) {
			interruptContextId = i;
			break;
		}
	}
	if (interruptContextId < 0) {
		// No empty slots for storing interruptContext
		return false;
	}
	BleInterruptContext& context = _bleInterruptContext[interruptContextId];
	context.eventHandler         = eventHandler;
	context.filled               = true;
	context.type                 = interruptType(eventType);

	// Also register interrupt on the microapp side
	interrupt_registration_t interrupt;
	interrupt.major          = CS_MICROAPP_SDK_TYPE_BLE;
	interrupt.minor          = interruptType(eventType);
	interrupt.handler        = handleBleInterrupt;
	microapp_sdk_result_t result = registerInterrupt(&interrupt);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// No empty interrupt slots available on microapp side
		// Remove interrupt context
		context.eventHandler = nullptr;
		context.filled       = false;
		return false;
	}

	// Finally, send a message to bluenet registering the interrupt
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = requestType(eventType);

	sendMessage();

	// Bluenet will set ack to true upon success
	if (bleRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// Undo local interrupt registration
		removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, interruptType(eventType));
		// Undo interrupt context
		context.eventHandler = nullptr;
		context.filled       = false;
		return false;
	}
	return true;
}

bool Ble::scan(bool withDuplicates) {
	if (_isScanning) {
		return true;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN_START;

	sendMessage();

	bool success = (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
	if (!success) {
		return false;
	}
	_isScanning = true;
	return true;
}

bool Ble::scanForName(const char* name, bool withDuplicates) {
	_activeFilter.type = BleFilterLocalName;
	_activeFilter.len  = strlen(name);
	memcpy(_activeFilter.name, name, _activeFilter.len);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForAddress(const char* address, bool withDuplicates) {
	_activeFilter.type    = BleFilterAddress;
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
	if (!_isScanning) {  // already not scanning
		return true;
	}

	// send a message to bluenet asking it to stop forwarding ads to microapp
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);

	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN_STOP;

	sendMessage();

	bool success = (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
	if (!success) {
		return false;
	}

	_activeFilter.type = BleFilterNone;  // reset filter
	_isScanning        = false;
	return true;
}

BleDevice Ble::available() {
	return _activeDevice;
}

bool Ble::filterScanEvent(BleDevice device) {
	if (!_isScanning) {  // return false by default if not scanning
		return false;
	}
	switch (_activeFilter.type) {
		case BleFilterAddress: {
			if (device._address != _activeFilter.address) {
				return false;
			}
			return true;
		}
		case BleFilterLocalName: {
			if (!device.hasLocalName()) {
				return false;
			}
			String deviceName = device.localName();
			if (deviceName.length() != _activeFilter.len) {
				return false;
			}
			// compare local name to name in filter
			if (memcmp(deviceName.c_str(), _activeFilter.name, _activeFilter.len) != 0) {
				return false;
			}
			return true;
		}
		case BleFilterUuid: {
			return device.findServiceDataUuid(_activeFilter.uuid);
		}
		case BleFilterNone: {
			// If no filter is set, we pass the filter by default
			return true;
		}
		default: {
			// Unknown filter type
			return false;
		}
	}
	return false;
}

BleFilter* Ble::getFilter() {
	return &_activeFilter;
}

MicroappSdkBleType Ble::requestType(BleEventType type) {
	switch (type) {
		case BLEDeviceScanned: return CS_MICROAPP_SDK_BLE_SCAN_REGISTER_INTERRUPT;
		case BLEConnected: return CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_CONNECT;
		case BLEDisconnected: return CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_DISCONNECT;
		default: return CS_MICROAPP_SDK_BLE_NONE;
	}
}

MicroappSdkBleType Ble::interruptType(BleEventType type) {
	switch (type) {
		case BLEDeviceScanned: return CS_MICROAPP_SDK_BLE_SCAN_SCANNED_DEVICE;
		case BLEConnected: return CS_MICROAPP_SDK_BLE_CONNECTION_CONNECTED;
		case BLEDisconnected: return CS_MICROAPP_SDK_BLE_CONNECTION_DISCONNECTED;
		default: return CS_MICROAPP_SDK_BLE_NONE;
	}
}