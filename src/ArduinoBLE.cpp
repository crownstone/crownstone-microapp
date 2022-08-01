#include <ArduinoBLE.h>

/*
 * An ordinary C function to keep the softInterrupt simple.
 */
int softInterruptBle(void* args, void* buf) {
	if (args == nullptr || buf == nullptr) {
		return ERR_MICROAPP_SOFT_INTERRUPT_NOT_REGISTERED;
	}

	microapp_ble_device_t* dev       = (microapp_ble_device_t*)buf;
	BleSoftInterruptContext* context = (BleSoftInterruptContext*)args;

	if (!context->eventHandler) {
		return ERR_MICROAPP_SOFT_INTERRUPT_NOT_REGISTERED;
	}

	// Based on the id (=type) of interrupt we will take action
	// This could probably also be done by checking dev->header.interruptId
	switch (context->id) {
		case BleEventDeviceScanned: {
			// Create temporary object on the stack
			BleDevice bleDevice = BleDevice(*dev);
			if (BLE.filterScanEvent(bleDevice)) {
				// Call the event handler with a copy of this object
				context->eventHandler(bleDevice);
			}
			break;
		}
		// TODO: implement handlers for other ble events
		default: {
			return ERR_MICROAPP_NOT_IMPLEMENTED;
		}
	}
	return ERR_MICROAPP_SUCCESS;
}

Ble::Ble() {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; i++) {
		_bleSoftInterruptContext[i].eventHandler = nullptr;
		_bleSoftInterruptContext[i].filled       = false;
	}
}

/*
 * Set softInterrupt, but not directly... We register a wrapper function that calls
 * this softInterrupt so we have the possibility to send a "softInterrupt end" command
 * after the softInterrupt has been executed.
 */
bool Ble::setEventHandler(BleEventType type, void (*eventHandler)(BleDevice)) {
	// Register the interrupt context locally
	int softInterruptContextId = -1;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (_bleSoftInterruptContext[i].filled == false) {
			softInterruptContextId = i;
			break;
		}
	}
	if (softInterruptContextId < 0) {
		// No empty slots for storing softInterruptContext
		return false;
	}
	BleSoftInterruptContext& context = _bleSoftInterruptContext[softInterruptContextId];
	context.eventHandler = eventHandler;
	context.filled = true;
	// There can be only one registered event handler per event type
	context.id = type;

	// Also register soft interrupt on the microapp side
	soft_interrupt_t softInterrupt;
	softInterrupt.id                = type;
	softInterrupt.type              = SOFT_INTERRUPT_TYPE_BLE;
	softInterrupt.softInterruptFunc = softInterruptBle;
	softInterrupt.arg               = &_bleSoftInterruptContext[softInterruptContextId];
	int result = registerSoftInterrupt(&softInterrupt);
	if (result != ERR_MICROAPP_SUCCESS) {
		// No empty interrupt slots available on microapp side
		// Remove soft interrupt context
		context.eventHandler = nullptr;
		context.id = 0;
		context.filled = false;
		return false;
	}

	// Finally, send a message to bluenet registering the soft interrupt
	uint8_t *payload            = getOutgoingMessagePayload();
	microapp_ble_cmd_t* ble_cmd = (microapp_ble_cmd_t*)(payload);
	ble_cmd->header.cmd         = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->header.ack         = false;
	ble_cmd->id                 = context.id;
	ble_cmd->opcode             = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;

	sendMessage();

	// Bluenet will set ack to true upon success
	if (!ble_cmd->header.ack) {
		// Undo local softInterrupt registration
		removeRegisteredSoftInterrupt(SOFT_INTERRUPT_TYPE_BLE, type);
		// Undo soft interrupt context
		context.eventHandler = nullptr;
		context.id = 0;
		context.filled = false;
		return false;
	}
	return true;
}

bool Ble::scan(bool withDuplicates) {
	if (_isScanning) {
		return true;
	}

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_ble_cmd_t* ble_cmd = (microapp_ble_cmd_t*)(payload);
	ble_cmd->header.ack         = false;
	ble_cmd->header.cmd         = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode             = CS_MICROAPP_COMMAND_BLE_SCAN_START;

	sendMessage();

	bool success = ble_cmd->header.ack;
	if (!success) {
		return success;
	}
	_isScanning = true;
	return success;
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
	uint8_t *payload = getOutgoingMessagePayload();
	microapp_ble_cmd_t* ble_cmd = (microapp_ble_cmd_t*)(payload);

	ble_cmd->header.ack = false;
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode     = CS_MICROAPP_COMMAND_BLE_SCAN_STOP;

	sendMessage();

	bool success = ble_cmd->header.ack;
	if (!success) {
		return success;
	}

	_activeFilter.type = BleFilterNone;  // reset filter
	_isScanning = false;

	return success;
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
			// TODO: refactor. Now filters ads of type service data with as
			// first element the filtered uuid, which is not the same as what
			// the official ArduinoBLE library does
			data_ptr_t serviceData;
			if (!device.findAdvertisementDataType(GapAdvType::ServiceData, &serviceData)) {
				return false;
			}
			uint16_t uuid = ((serviceData.data[1] << 8) | serviceData.data[0]);
			if (uuid != _activeFilter.uuid) {
				return false;
			}
			return true;
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
