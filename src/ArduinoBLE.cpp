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
	// Based on the type of event we will take action
	switch (bleInterrupt->type) {
		case CS_MICROAPP_SDK_BLE_SCAN: {
			// for scan type, only interrupt type is EVENT_SCAN
			switch (bleInterrupt->scan.type) {
				case CS_MICROAPP_SDK_BLE_SCAN_EVENT_SCAN: {
					// Get the interrupt context with the eventHandler
					BleInterruptContext context;
					getInterruptContext(BLEDeviceScanned, context);
					// Create temporary object on the stack
					// Lifetime of bleDevice is only as long as the lifetime of the interrupt stack
					BleDevice bleDevice = BleDevice(&bleInterrupt->scan.eventScan);
					if (filterScanEvent(bleDevice)) {
						context.eventHandler(bleDevice);
					}
					return CS_MICROAPP_SDK_ACK_SUCCESS;
				}
				default: {
					return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
				}
			}
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL: {
			// TODO: implement
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL: {
			// TODO: implement
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
		default: {
			// For the UUID_REGISTER type, no interrupt handling is defined
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

/*
 * Set event handler provided by user, but not directly...
 * We register a wrapper function that calls the passed handler
 */
bool Ble::setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice)) {

	// Register the interrupt context locally
	microapp_sdk_result_t result = setInterruptContext(eventType, eventHandler);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}

	// Also register interrupt on the microapp side,
	// with an indirect handler
	interrupt_registration_t interrupt;
	interrupt.type    = CS_MICROAPP_SDK_TYPE_BLE;
	interrupt.id      = getBleType(eventType);
	interrupt.handler = handleBleInterrupt;
	result            = registerInterrupt(&interrupt);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// No empty interrupt slots available on microapp side
		// Remove interrupt context
		removeInterruptContext(eventType);
		return false;
	}

	// Finally, send a message to bluenet registering the interrupt
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = getBleType(eventType);
	switch (bleRequest->type) {
		case CS_MICROAPP_SDK_BLE_SCAN:
			bleRequest->scan.type = CS_MICROAPP_SDK_BLE_SCAN_REGISTER_INTERRUPT;
			break;
		case CS_MICROAPP_SDK_BLE_CENTRAL:
			bleRequest->central.type = CS_MICROAPP_SDK_BLE_CENTRAL_REGISTER_INTERRUPT;
			break;
		case CS_MICROAPP_SDK_BLE_PERIPHERAL:
			bleRequest->peripheral.type = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_REGISTER_INTERRUPT;
			break;
		default:
			// unsupported ble type for setting event handler
			removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, getBleType(eventType));
			removeInterruptContext(eventType);
			return false;
	}

	sendMessage();

	// Bluenet will set ack to true upon success
	if (bleRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// Undo local interrupt registration
		removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, getBleType(eventType));
		// Undo interrupt context
		removeInterruptContext(eventType);
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
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN;
	bleRequest->scan.type          = CS_MICROAPP_SDK_BLE_SCAN_START;

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
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN;
	bleRequest->scan.type          = CS_MICROAPP_SDK_BLE_SCAN_STOP;

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

MicroappSdkBleType Ble::getBleType(BleEventType eventType) {
	switch (eventType) {
		case BLEDeviceScanned:
			return CS_MICROAPP_SDK_BLE_SCAN;
		case BLEConnected:
			// todo: can also be peripheral?
			return CS_MICROAPP_SDK_BLE_CENTRAL;
		case BLEDisconnected:
			// todo: can also be peripheral?
			return CS_MICROAPP_SDK_BLE_CENTRAL;
		default: return CS_MICROAPP_SDK_BLE_NONE;
	}
}

microapp_sdk_result_t Ble::setInterruptContext(BleEventType eventType, void (*eventHandler)(BleDevice)) {
	int interruptContextId = -1;
	for (int i = 0; i < MAX_BLE_INTERRUPT_REGISTRATIONS; ++i) {
		if (_bleInterruptContext[i].filled == false) {
			interruptContextId = i;
			break;
		}
		if (_bleInterruptContext[i].eventType == eventType) {
			return CS_MICROAPP_SDK_ACK_ERR_ALREADY_EXISTS;
		}
	}
	if (interruptContextId < 0) {
		// No empty slots for storing interruptContext
		return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
	}
	BleInterruptContext& context = _bleInterruptContext[interruptContextId];
	context.eventHandler         = eventHandler;
	context.filled               = true;
	context.type                 = getBleType(eventType);
	context.eventType            = eventType;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t Ble::getInterruptContext(BleEventType eventType, BleInterruptContext& context) {
	for (int i = 0; i < MAX_BLE_INTERRUPT_REGISTRATIONS; ++i) {
		if (_bleInterruptContext[i].filled == false) {
			continue;
		}
		if (_bleInterruptContext[i].eventType == eventType) {
			if (_bleInterruptContext[i].eventHandler == nullptr) {
				return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
			}
			context = _bleInterruptContext[i];
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

microapp_sdk_result_t Ble::removeInterruptContext(BleEventType eventType) {
	for (int i = 0; i < MAX_BLE_INTERRUPT_REGISTRATIONS; ++i) {
		if (_bleInterruptContext[i].filled == false) {
			continue;
		}
		if (_bleInterruptContext[i].eventType == eventType) {
			_bleInterruptContext[i].eventHandler = nullptr;
			_bleInterruptContext[i].type         = CS_MICROAPP_SDK_BLE_NONE;
			_bleInterruptContext[i].filled       = false;
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}