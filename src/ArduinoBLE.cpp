#include <Arduino.h>
#include <ArduinoBLE.h>

/*
 * An ordinary C function. Calls internal handler
 */
microapp_sdk_result_t handleBleInterrupt(void* interrupt) {
	if (interrupt == nullptr) {
		return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
	}
	// Size is not checked because it is variable
	microapp_sdk_ble_t* bleInterrupt = reinterpret_cast<microapp_sdk_ble_t*>(interrupt);
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
			MacAddress address(scanInterrupt->eventScan.address.address, MAC_ADDRESS_LENGTH, scanInterrupt->eventScan.address.type);
			if (!matchesFilter(scan, address)) {
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			// Copy the scan data into the _scanDevice
			rssi_t rssi = scanInterrupt->eventScan.rssi;
			_scanDevice = BleDevice(scan, address, rssi);
			// Get the event handler registration
			BleEventHandlerRegistration registration;
			microapp_sdk_result_t result = getBleEventHandlerRegistration(BLEDeviceScanned, registration);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				// Most likely no event handler was registered
				// We do not return an error but silently return
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			// Call the event handler
			DeviceEventHandler handler = (DeviceEventHandler)registration.eventHandler;
			handler(_scanDevice);
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

microapp_sdk_result_t Ble::handleCentralEvent(microapp_sdk_ble_central_t* central) {
	if (!_device._flags.flags.isPeripheral) {
		// Other device is not a peripheral
		// Thus, we do nothing with central events
		// First scan for a peripheral
		return CS_MICROAPP_SDK_ACK_ERR_DISABLED;
	}
	microapp_sdk_result_t result;
	switch (central->type) {
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_CONNECT: {
			if (central->eventConnect.result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				_device._asyncResult = BleAsyncFailure;
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			_device.onConnect(central->connectionHandle);
			// check for event handlers
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEConnected, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				DeviceEventHandler handler = (DeviceEventHandler)registration.eventHandler;
				handler(_device);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCONNECT: {
			_device.onDisconnect();
			// clean up own member variables as well
			_remoteServiceCount = 0;
			_remoteCharacteristicCount = 0;
			// check for event handlers
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEDisconnected, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				DeviceEventHandler handler = (DeviceEventHandler)registration.eventHandler;
				handler(_device);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCOVER: {
			if (central->eventDiscover.valueHandle == 0) {
				// discovered a service
				if (_remoteServiceCount >= MAX_REMOTE_SERVICES) {
					return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
				}
				BleService service(&central->eventDiscover.uuid);
				_remoteServices[_remoteServiceCount] = service;
				// add to device
				result = _device.addDiscoveredService(&_remoteServices[_remoteServiceCount]);
				if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
					return result;
				}
				_remoteServiceCount++;
			}
			else {
				// discovered a characteristic
				if (_remoteCharacteristicCount >= MAX_REMOTE_CHARACTERISTICS) {
					return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
				}
				uint8_t properties = 0;
				if (central->eventDiscover.options.read) {
					properties |= BleCharacteristicProperties::BLERead;
				}
				if (central->eventDiscover.options.writeNoResponse) {
					properties |= BleCharacteristicProperties::BLEWriteWithoutResponse;
				}
				if (central->eventDiscover.options.write) {
					properties |= BleCharacteristicProperties::BLEWrite;
				}
				if (central->eventDiscover.options.notify) {
					properties |= BleCharacteristicProperties::BLENotify;
				}
				if (central->eventDiscover.options.indicate) {
					properties |= BleCharacteristicProperties::BLEIndicate;
				}
				BleCharacteristic characteristic(&central->eventDiscover.uuid, properties);
				characteristic._valueHandle = central->eventDiscover.valueHandle;
				characteristic._cccdHandle = central->eventDiscover.cccdHandle;
				_remoteCharacteristics[_remoteCharacteristicCount] = characteristic;
				// add to device
				Uuid serviceUuid(central->eventDiscover.serviceUuid.uuid, central->eventDiscover.serviceUuid.type);
				result = _device.addDiscoveredCharacteristic(
						&_remoteCharacteristics[_remoteCharacteristicCount], serviceUuid);
				if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
					return result;
				}
				_remoteCharacteristicCount++;
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCOVER_DONE: {
			if (central->eventDiscoverDone.result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				_device._asyncResult = BleAsyncFailure;
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}
			_device.onDiscoverDone();
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_WRITE: {
			BleCharacteristic* characteristic;
			result = _device.getCharacteristic(central->eventWrite.handle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = (microapp_sdk_result_t)central->eventWrite.result;
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				// set async result
				characteristic->_asyncResult = BleAsyncFailure;
				return result;
			}
			// set async result so blocking function may return
			result = characteristic->onRemoteWritten();
			return result;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_READ: {
			BleCharacteristic* characteristic;
			result = _device.getCharacteristic(central->eventRead.valueHandle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = (microapp_sdk_result_t)central->eventRead.result;
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				characteristic->_asyncResult = BleAsyncFailure;
				return result;
			}
			result = characteristic->onRemoteRead(&central->eventRead);
			return result;
		}
		case CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_NOTIFICATION: {
			BleCharacteristic* characteristic;
			result = _device.getCharacteristic(central->eventNotification.valueHandle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = characteristic->onRemoteNotification(&central->eventNotification);
			return result;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

microapp_sdk_result_t Ble::handlePeripheralEvent(microapp_sdk_ble_peripheral_t* peripheral) {
	microapp_sdk_result_t result;
	switch (peripheral->type) {
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_CONNECT: {
			// Build central device
			_device = BleDevice(
					MacAddress(peripheral->eventConnect.address.address, MAC_ADDRESS_LENGTH, peripheral->eventConnect.address.type));
			_device.onConnect(peripheral->connectionHandle);
			// check for event handlers
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEConnected, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				DeviceEventHandler handler = (DeviceEventHandler)registration.eventHandler;
				handler(_device);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_DISCONNECT: {
			_device.onDisconnect();
			// check for event handlers
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEDisconnected, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				DeviceEventHandler handler = (DeviceEventHandler)registration.eventHandler;
				handler(_device);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_WRITE: {
			BleCharacteristic* characteristic;
			result = getLocalCharacteristic(peripheral->handle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = characteristic->onLocalWritten(&peripheral->eventWrite);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEWritten, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				CharacteristicEventHandler handler = (CharacteristicEventHandler)registration.eventHandler;
				handler(_device, *characteristic);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_READ: {
			// Not implemented
			return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_SUBSCRIBE: {
			BleCharacteristic* characteristic;
			result = getLocalCharacteristic(peripheral->handle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = characteristic->onLocalSubscribed();
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLESubscribed, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				CharacteristicEventHandler handler = (CharacteristicEventHandler)registration.eventHandler;
				handler(_device, *characteristic);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_UNSUBSCRIBE: {
			BleCharacteristic* characteristic;
			result = getLocalCharacteristic(peripheral->handle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = characteristic->onLocalUnsubscribed();
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			BleEventHandlerRegistration registration;
			result = getBleEventHandlerRegistration(BLEUnsubscribed, registration);
			if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
				// call callback
				CharacteristicEventHandler handler = (CharacteristicEventHandler)registration.eventHandler;
				handler(_device, *characteristic);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		case CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_NOTIFICATION_DONE: {
			BleCharacteristic* characteristic;
			result = getLocalCharacteristic(peripheral->handle, &characteristic);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return result;
			}
			result = characteristic->onLocalNotificationDone();
			return result;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

// Only defined for peripheral
microapp_sdk_result_t Ble::getLocalCharacteristic(uint16_t handle, BleCharacteristic** characteristic) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	for (uint8_t i = 0; i < _localServiceCount; i++) {
		microapp_sdk_result_t result = _localServices[i]->getCharacteristic(handle, characteristic);
		if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

bool Ble::begin() {
	// send a message to bluenet requesting own address
	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->type               = CS_MICROAPP_SDK_BLE_MAC;

	sendMessage();

	bool success = (bleRequest->header.ack == CS_MICROAPP_SDK_ACK_SUCCESS);
	if (!success) {
		return false;
	}
	_address = MacAddress(bleRequest->requestMac.address.address, MAC_ADDRESS_LENGTH, bleRequest->requestMac.address.type);
	_flags.flags.initialized = true;
	return true;
}

void Ble::end() {
	_address = MacAddress();
	_scanDevice = BleDevice();
	_device = BleDevice();
	_flags.asInt = 0;
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
bool Ble::setEventHandler(BleEventType eventType, DeviceEventHandler eventHandler) {
	microapp_sdk_result_t result;
	switch (eventType) {
		case BLEDeviceScanned: {
			result = registerBleEventHandler(eventType, (BleEventHandler)eventHandler);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return false;
			}
			if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_SCAN)) {
				result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_SCAN);
				if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
					removeBleEventHandlerRegistration(eventType);
					return false;
				}
			}
			return true;
		}
		case BLEConnected:
		case BLEDisconnected: {
			// Register the event handler for both central and peripheral role
			result = registerBleEventHandler(eventType, (BleEventHandler)eventHandler);
			if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return false;
			}
			if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_CENTRAL)) {
				result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_CENTRAL);
				if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
					removeBleEventHandlerRegistration(eventType);
					return false;
				}
			}
			if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL)) {
				result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL);
				if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
					removeBleEventHandlerRegistration(eventType);
					return false;
				}
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

bool Ble::disconnect(uint32_t timeout) {
	if (!_device.connected() || !_flags.flags.initialized) {
		return false;
	}
	else {
		return _device.disconnect(timeout);
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
	if (_localServiceCount >= MAX_LOCAL_SERVICES || !_flags.flags.initialized) {
		return;
	}
	microapp_sdk_result_t result;
	if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL)) {
		result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_PERIPHERAL);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return;
		}
	}
	result = service.addLocalService();
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return;
	}
	_localServices[_localServiceCount] = &service;
	_localServiceCount++;
}

BleDevice& Ble::central() {
	if (_device.connected() && _device._flags.flags.isCentral) {
		return _device;
	}
	else {
		// Reset device
		_device = BleDevice();
		return _device;
	}
}

bool Ble::scan(bool withDuplicates) {
	if (!_flags.flags.initialized) {
		return false;
	}
	// Reset existing _scanDevice
	_scanDevice = BleDevice();
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
	// Reset existing _scanDevice
	_scanDevice = BleDevice();

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
	if (!_flags.flags.initialized || !_flags.flags.isScanning) {
		// Reset device
		_device = BleDevice();
		return _device;
	}
	if (!_scanDevice) {
		// Reset device
		_device = BleDevice();
		return _device;
	}
	// Set main (persistent) device as the latest scanned device
	_device = _scanDevice;
	// Reset scan device
	_scanDevice = BleDevice();
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
			// in the ad, name is not zero terminated, so use length of name provided by user
			if (memcmp(localName.data, _scanFilter.localName, _scanFilter.localNameLen) != 0) {
				return false;
			}
			return true;
		}
		case BleFilterUuid: {
			if (_scanFilter.uuid.custom()) {
				return false;
			}
			return scan.hasServiceUuid(_scanFilter.uuid.uuid16());
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

microapp_sdk_result_t registerBleEventHandler(BleEventType eventType, BleEventHandler eventHandler) {
	int eventHandlerRegistrationIndex = -1;
	for (int i = 0; i < BLE.MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (BLE._bleEventHandlerRegistration[i].filled == false) {
			eventHandlerRegistrationIndex = i;
			break;
		}
		if (BLE._bleEventHandlerRegistration[i].eventType == eventType) {
			return CS_MICROAPP_SDK_ACK_ERR_ALREADY_EXISTS;
		}
	}
	if (eventHandlerRegistrationIndex < 0) {
		// No empty slots for storing event handler
		return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
	}
	BleEventHandlerRegistration& registration = BLE._bleEventHandlerRegistration[eventHandlerRegistrationIndex];
	registration.eventHandler                 = eventHandler;
	registration.filled                       = true;
	registration.eventType                    = eventType;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

microapp_sdk_result_t getBleEventHandlerRegistration(
		BleEventType eventType, BleEventHandlerRegistration& registration) {
	for (int i = 0; i < BLE.MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (BLE._bleEventHandlerRegistration[i].filled == false) {
			continue;
		}
		if (BLE._bleEventHandlerRegistration[i].eventType == eventType) {
			if (BLE._bleEventHandlerRegistration[i].eventHandler == nullptr) {
				// Should not happen, if filled is true the eventHandler should be valid
				return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
			}
			registration = BLE._bleEventHandlerRegistration[i];
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

microapp_sdk_result_t removeBleEventHandlerRegistration(BleEventType eventType) {
	for (int i = 0; i < BLE.MAX_BLE_EVENT_HANDLER_REGISTRATIONS; ++i) {
		if (BLE._bleEventHandlerRegistration[i].filled == false) {
			continue;
		}
		if (BLE._bleEventHandlerRegistration[i].eventType == eventType) {
			BLE._bleEventHandlerRegistration[i].eventHandler = nullptr;
			BLE._bleEventHandlerRegistration[i].filled       = false;
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

bool registeredBleInterrupt(MicroappSdkBleType bleType) {
	switch (bleType) {
		case CS_MICROAPP_SDK_BLE_SCAN: return BLE._flags.flags.registeredScanInterrupts;
		case CS_MICROAPP_SDK_BLE_CENTRAL: return BLE._flags.flags.registeredCentralInterrupts;
		case CS_MICROAPP_SDK_BLE_PERIPHERAL: return BLE._flags.flags.registeredPeripheralInterrupts;
		default: return false;
	}
}

microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType bleType) {
	if (registeredBleInterrupt(bleType)) {
		return CS_MICROAPP_SDK_ACK_ERR_ALREADY_EXISTS;
	}
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
			bleRequest->central.type = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_REGISTER_INTERRUPT;
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
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// Undo local interrupt registration
		removeInterruptRegistration(CS_MICROAPP_SDK_TYPE_BLE, bleType);
		return result;
	}
	switch (bleType) {
		case CS_MICROAPP_SDK_BLE_SCAN: BLE._flags.flags.registeredScanInterrupts = true; break;
		case CS_MICROAPP_SDK_BLE_CENTRAL: BLE._flags.flags.registeredCentralInterrupts = true; break;
		case CS_MICROAPP_SDK_BLE_PERIPHERAL: BLE._flags.flags.registeredPeripheralInterrupts = true; break;
		default:
			// would have returned early earlier this function
			break;
	}
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}
