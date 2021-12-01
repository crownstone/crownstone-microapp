#include <ArduinoBLE.h>

// Filters and forwards the bluenet scanned device event interrupt to the user callback
void handleScanEvent(microapp_ble_dev_t dev)
{
	BleFilter* filter = BLE.getFilter();
	switch (filter->filterType) {
		case BleFilterAddress: {
			//Serial.print("Scanned device MAC address "); Serial.println(dev.addr, sizeof(dev.addr));
			if (memcmp(dev.addr,filter->address.byte,MAC_ADDRESS_LENGTH) != 0) return;
			break;
		}
		case BleFilterLocalName: {
			uint8_t type = dev.data[1];
			if (type != 0x09) return; // Not a complete local name ad
			char * deviceName = (char*) &dev.data[2];
			if ((dev.dlen - 2) != strlen(filter->completeLocalName)) return; // device name and filter name don't have same length
			if (memcmp(deviceName,filter->completeLocalName,dev.dlen - 2) != 0) return; // local name doesn't match filter name
			break;
		}
		case BleFilterServiceData: {
			uint8_t type = dev.data[1];
			if (type != 0x16) return; // Not a service data ad
			uint16_t uuid = ((dev.data[3] << 8) | dev.data[2]);
			if (uuid != filter->uuid) return; // service data uuid does not match filter uuid
			break;
		}
		case BleFilterNone:
		default:
			break;
	}
	// now call the user registered callback
	void (*callback_func)(microapp_ble_dev_t) = (void (*)(microapp_ble_dev_t)) BLE._scanned_device_callback;
	callback_func(dev);
}


void Ble::setEventHandler(BleEventHandlerType type, void (*isr)(microapp_ble_dev_t))
{
	Serial.println("Setting event handler");

	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;
	ble_cmd->callback = (uintptr_t)(handleScanEvent);
	_scanned_device_callback = (uintptr_t)(isr);

	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);
}

bool Ble::scan()
{
	if (_isScanning) return true;

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

void Ble::stopScan()
{
	if (!_isScanning) return;

	Serial.println("Stopping BLE device scanning");

	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_STOP;
	global_msg.length = sizeof(microapp_ble_cmd_t);

	_isScanning = false;
	// TODO: check for return message from bluenet
	sendMessage(&global_msg);
}

void Ble::addFilter(BleFilter filter)
{
	Serial.println("Setting filter");
	_filter = filter;
}

void Ble::removeFilter()
{
	Serial.println("Removing filter");
	_filter.filterType = BleFilterNone;
}

BleFilter* Ble::getFilter()
{
	return &_filter;
}
