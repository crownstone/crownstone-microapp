#include <Arduino.h>
#include <ArduinoBLE.h>

/*
 * An ordinary C function. Calls internal handler
 */
microapp_sdk_result_t handleBleInterrupt(void* buf) {
	if (buf == nullptr) {
		return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
	}
	microapp_sdk_ble_t* bleInterrupt = reinterpret_cast<microapp_sdk_ble_t*>(buf);
	return BLE.handleEvent(bleInterrupt);
}

/*
 * Internal interrupt handler.
 * Checks type of interrupt and takes appropriate action
 */
microapp_sdk_result_t Ble::handleEvent(microapp_sdk_ble_t* bleInterrupt) {
	// Based on the type of event we will take action
	switch (bleInterrupt->type) {
		case CS_MICROAPP_SDK_BLE_SCAN: {
			return handleScanEvent(&bleInterrupt->scan);
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL: {
			return handleCentralEvent(&bleInterrupt->central);
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL: {
			return handlePeripheralEvent(&bleInterrupt->peripheral);
		}
		default: {
			// For the other types, no interrupt handling is defined
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

microapp_sdk_result_t Ble::handleScanEvent(microapp_sdk_ble_scan_t* scanInterrupt) {
	// for scan type, only interrupt type is EVENT_SCAN for now
	switch (scanInterrupt->type) {
		case CS_MICROAPP_SDK_BLE_SCAN_EVENT_SCAN: {
			if (!_flags.flags.isScanning) {
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			// Apply a wrapper BleScan (no copy) to parse scan data
			BleScan scan(scanInterrupt->eventScan.data, scanInterrupt->eventScan.size);
			MacAddress address(scanInterrupt->eventScan.address.address);
			if (!matchesFilter(scan, address)) {
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			// Copy the scan data into the _scanDevice
			rssi_t rssi = scanInterrupt->eventScan.rssi;
			_scanDevice = BleDevice(scan, address, rssi);
			// Get the event handler registration
			BleEventHandlerRegistration handlerRegistration;
			microapp_sdk_result_t result = getEventHandlerRegistration(BLEDeviceScanned, handlerRegistration);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				// Most likely no event handler was registered
				// We do not return an error but silently return
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			// Call the event handler
			handlerRegistration.eventHandler(_scanDevice);
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

microapp_sdk_result_t Ble::handleCentralEvent(microapp_sdk_ble_central_t* central) {
	// check for event handlers if connected or disconnected event
	return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
}

microapp_sdk_result_t Ble::handlePeripheralEvent(microapp_sdk_ble_peripheral_t* peripheral) {
	// check for event handlers if connected or disconnected event
	return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
}

bool Ble::begin() {
	// todo: make roundtrip to bluenet to request own ble address
	_flags.flags.initialized = true;
	return true;
}

void Ble::end() {
	_flags.flags.initialized = false;
	return;
}

void Ble::poll(int timeout) {
	if (!_flags.flags.initialized) {
		return;
	}
	if (timeout == 0) {
		return;
	}
	delay(timeout);
}

/*
 * Set event handler provided by user, but not directly...
 * We register a wrapper function that calls the passed handler
 */
bool Ble::setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice)) {
	microapp_sdk_result_t result;
	switch (eventType) {
		case BLEDeviceScanned: {
			result = registerEventHandler(eventType, eventHandler);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return false;
			}
			result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_SCAN);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				removeEventHandlerRegistration(eventType);
				return false;
			}
			return true;
		}
		case BLEConnected:
		case BLEDisconnected: {
			// Register the event handler for both central and peripheral role
			result = registerEventHandler(eventType, eventHandler);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return false;
			}
			result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_CENTRAL);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				removeEventHandlerRegistration(eventType);
				return false;
			}
			result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				removeEventHandlerRegistration(eventType);
				// Also remove interrupt registration for central
				// if the interrupt registration for peripheral fails:
				// It's both or none
				removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
				return false;
			}
			return true;
		}
		default: {
			// Undefined
			return false;
		}
	}
}

bool Ble::connected() {
	if (!_flags.flags.initialized) {
		return false;
	}
	return _device.connected();
}

bool Ble::disconnect() {
	if (!_device.connected() || !_flags.flags.initialized) {
		return false;
	}
	else {
		return _device.disconnect();
	}
}

String Ble::address() {
	if (!_flags.flags.initialized) {
		return String(nullptr);
	}
	return String(_address.string());
}

int8_t Ble::rssi() {
	// If not connected return 127 as per the official arduino library
	if (!_device.connected() || !_flags.flags.initialized) {
		return 127;
	}
	else {
		return _device.rssi();
	}
}

void Ble::addService(BleService& service) {
	if (_serviceCount >= MAX_SERVICES || !_flags.flags.initialized) {
		return;
	}
	microapp_sdk_result_t result = service.add();
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return;
	}
	_services[_serviceCount] = &service;
	_serviceCount++;
}

BleDevice& Ble::central() {
	if (_device.connected() && _device._flags.flags.isCentral) {
		return _device;
	}
	else {
		static BleDevice empty;
		return empty;
	}
}

bool Ble::scan(bool withDuplicates) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (_flags.flags.isScanning) {
		return true;
	}

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN;
	bleRequest->scan.type          = CS_MICROAPP_SDK_BLE_SCAN_REQUEST_START;

	sendMessage();

	bool success = (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
	if (!success) {
		return false;
	}
	_flags.flags.isScanning = true;
	return true;
}

bool Ble::scanForName(const char* name, bool withDuplicates) {
	if (!_flags.flags.initialized) {
		return false;
	}
	_scanFilter.type         = BleFilterLocalName;
	_scanFilter.localNameLen = strlen(name);
	if (_scanFilter.localNameLen > MAX_BLE_ADV_DATA_LENGTH) {
		_scanFilter.localNameLen = MAX_BLE_ADV_DATA_LENGTH;
	}

	memcpy(_scanFilter.localName, name, _scanFilter.localNameLen);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForAddress(const char* address, bool withDuplicates) {
	if (!_flags.flags.initialized) {
		return false;
	}
	_scanFilter.type    = BleFilterAddress;
	_scanFilter.address = MacAddress(address);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForUuid(const char* uuid, bool withDuplicates) {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (strlen(uuid) != UUID_16BIT_STRING_LENGTH) {
		return false;
	}
	_scanFilter.type = BleFilterUuid;
	_scanFilter.uuid = Uuid(uuid);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::stopScan() {
	if (!_flags.flags.initialized) {
		return false;
	}
	if (!_flags.flags.isScanning) {  // already not scanning
		return true;
	}

	// send a message to bluenet asking it to stop forwarding ads to microapp
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);

	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_SCAN;
	bleRequest->scan.type          = CS_MICROAPP_SDK_BLE_SCAN_REQUEST_STOP;

	sendMessage();

	bool success = (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
	if (!success) {
		return false;
	}

	_scanFilter.type        = BleFilterNone;  // reset filter
	_flags.flags.isScanning = false;
	return true;
}

BleDevice& Ble::available() {
	if (!_flags.flags.initialized) {
		static BleDevice empty;
		return empty;
	}
	// Set main (persistent) device as the latest scanned device
	_device = _scanDevice;
	// Devices called via available() are peripheral devices
	_device._flags.flags.isPeripheral = true;
	return _device;
}

bool Ble::matchesFilter(BleScan scan, MacAddress address) {
	switch (_scanFilter.type) {
		case BleFilterAddress: {
			if (address != _scanFilter.address) {
				return false;
			}
			return true;
		}
		case BleFilterLocalName: {
			ble_ad_t localName = scan.localName();
			if (localName.len == 0) {
				return false;
			}
			if (localName.len != _scanFilter.localNameLen) {
				return false;
			}
			// compare local name to name in filter
			if (memcmp(localName.data, _scanFilter.localName, _scanFilter.localNameLen) != 0) {
				return false;
			}
			return true;
		}
		case BleFilterUuid: {
			if (_scanFilter.uuid.length() != UUID_16BIT_BYTE_LENGTH) {
				return false;
			}
			return scan.hasServiceDataUuid(_scanFilter.uuid.uuid16());
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

microapp_sdk_result_t Ble::registerBleInterrupt(MicroappSdkBleType bleType) {
	// Register interrupt on the microapp side,
	// with an indirect handler
	interrupt_registration_t interrupt;
	interrupt.type               = CS_MICROAPP_SDK_TYPE_BLE;
	interrupt.id                 = bleType;
	interrupt.handler            = handleBleInterrupt;
	microapp_sdk_result_t result = registerInterrupt(&interrupt);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// No empty interrupt slots available on microapp side
		return result;
	}

	// Send a message to bluenet registering the interrupt
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type               = bleType;
	switch (bleType) {
		case CS_MICROAPP_SDK_BLE_SCAN:
			bleRequest->scan.type = CS_MICROAPP_SDK_BLE_SCAN_REQUEST_REGISTER_INTERRUPT;
			break;
		case CS_MICROAPP_SDK_BLE_CENTRAL:
			bleRequest->central.type = CS_MICROAPP_SDK_BLE_CENTRAL_REGISTER_INTERRUPT;
			break;
		case CS_MICROAPP_SDK_BLE_PERIPHERAL:
			bleRequest->peripheral.type = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_REGISTER_INTERRUPT;
			break;
		default:
			// Unsupported ble type for setting event handler
			// Undo local interrupt registration
			removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, bleType);
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}

	sendMessage();

	// Bluenet will set ack to success upon success
	result = (microapp_sdk_result_t) bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// Undo local interrupt registration
		removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, bleType);
		return result;
	}
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t Ble::registerEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice)) {
	int eventHandlerRegistrationIndex = -1;
	for (int i = 0; i < MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (_bleEventHandlerRegistration[i].filled == false) {
			eventHandlerRegistrationIndex = i;
			break;
		}
		if (_bleEventHandlerRegistration[i].eventType == eventType) {
			return CS_MICROAPP_SDK_ACK_ERR_ALREADY_EXISTS;
		}
	}
	if (eventHandlerRegistrationIndex < 0) {
		// No empty slots for storing event handler
		return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
	}
	BleEventHandlerRegistration& registration = _bleEventHandlerRegistration[eventHandlerRegistrationIndex];
	registration.eventHandler                 = eventHandler;
	registration.filled                       = true;
	registration.eventType                    = eventType;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t Ble::getEventHandlerRegistration(
		BleEventType eventType, BleEventHandlerRegistration& registration) {
	for (int i = 0; i < MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (_bleEventHandlerRegistration[i].filled == false) {
			continue;
		}
		if (_bleEventHandlerRegistration[i].eventType == eventType) {
			if (_bleEventHandlerRegistration[i].eventHandler == nullptr) {
				return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
			}
			registration = _bleEventHandlerRegistration[i];
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

microapp_sdk_result_t Ble::removeEventHandlerRegistration(BleEventType eventType) {
	for (int i = 0; i < MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (_bleEventHandlerRegistration[i].filled == false) {
			continue;
		}
		if (_bleEventHandlerRegistration[i].eventType == eventType) {
			_bleEventHandlerRegistration[i].eventHandler = nullptr;
			_bleEventHandlerRegistration[i].filled       = false;
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}