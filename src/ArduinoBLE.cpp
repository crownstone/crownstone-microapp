#include <ArduinoBLE.h>

Ble::Ble() {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; i++) {
		_bleSoftInterruptContext[i].eventHandler = nullptr;
		_bleSoftInterruptContext[i].filled        = false;
	}
}

bool Ble::filterScanEvent(BleDevice device) {
	switch (_activeFilter.type) {
		case BleFilterAddress: {
			if (device._address != _activeFilter.address) {
				return false;
			}
			break;
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
			break;
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
			break;
		}
		case BleFilterNone:
		default: break;
	}
	return true;
}

/*
 * An ordinary C function to keep the softInterrupt simple.
 */
void softInterruptBle(void* args, void* buf) {
	microapp_ble_device_t* dev       = (microapp_ble_device_t*)buf;
	BleSoftInterruptContext* context = (BleSoftInterruptContext*)args;

	if (!context->eventHandler) {
		uint8_t *payload = getOutgoingMessagePayload();
		//io_buffer_t* buffer = getOutgoingMessageBuffer();
		microapp_cmd_t* cmd = (microapp_cmd_t*)(payload);
		cmd->cmd            = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_ERROR;
		cmd->id             = context->id;
		sendMessage();
		return;
	}

	// Create temporary object on the stack
	BleDevice bleDevice = BleDevice(*dev);
	// Call the event handler with a copy of this object
	context->eventHandler(bleDevice);

	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t* buffer = getOutgoingMessageBuffer();
	microapp_cmd_t* cmd = (microapp_cmd_t*)(payload);
	cmd->cmd            = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_END;
	cmd->id             = context->id;
	sendMessage();
}

/*
 * Set softInterrupt, but not directly... We register a wrapper function that calls
 * this softInterrupt so we have the possibility to send a "softInterrupt end" command
 * after the softInterrupt has been executed.
 */
void Ble::setEventHandler(BleEventType type, void (*eventHandler)(BleDevice)) {
	int softInterruptContextId = -1;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (_bleSoftInterruptContext[i].filled == false) {
			softInterruptContextId = i;
			break;
		}
	}
	if (softInterruptContextId < 0) {
		Serial.println("No space for new event handler");
		return;
	}
	BleSoftInterruptContext& context = _bleSoftInterruptContext[softInterruptContextId];

	Serial.println("Setting event handler");
	// TODO: do something with type. For now assume type is BleEventDeviceScanned
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t* buffer         = getOutgoingMessageBuffer();
	microapp_ble_cmd_t* ble_cmd = (microapp_ble_cmd_t*)(payload);
	ble_cmd->header.cmd         = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode             = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;

	// Set identifier now to 0 (assuming a single softInterrupt)
	ble_cmd->id = 0;

	context.eventHandler = eventHandler;
	context.filled        = true;
	context.id            = ble_cmd->id;

	soft_interrupt_t softInterrupt;
	softInterrupt.id            = ble_cmd->id;
	softInterrupt.type          = SOFT_INTERRUPT_TYPE_BLE;
	softInterrupt.softInterruptFunc = softInterruptBle;
	softInterrupt.arg           = &_bleSoftInterruptContext[softInterruptContextId];
	registerSoftInterrupt(&softInterrupt);

	sendMessage();
}

bool Ble::scan(bool withDuplicates) {
	if (_isScanning) {
		return true;
	}

	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t* buffer         = getOutgoingMessageBuffer();
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

	// Serial.println("Stopping BLE device scanning");

	_activeFilter.type = BleFilterNone;  // reset filter

	// send a message to bluenet commanding it to stop forwarding ads to
	// microapp
	// microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_buf_out;
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t* buffer         = getOutgoingMessageBuffer();
	microapp_ble_cmd_t* ble_cmd = (microapp_ble_cmd_t*)(payload);

	ble_cmd->header.ack = false;
	ble_cmd->header.cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode     = CS_MICROAPP_COMMAND_BLE_SCAN_STOP;

	sendMessage();

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
