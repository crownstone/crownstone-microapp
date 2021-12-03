#include <ArduinoBLE.h>

// Wrapper for handleScanEvent since class functions can't be registered as a callback
void handleScanEventWrapper(microapp_ble_dev_t dev)
{
	BLE.handleScanEvent(dev);
}

// Filters and forwards the bluenet scanned device event interrupt to the user callback
void Ble::handleScanEvent(microapp_ble_dev_t dev)
{
	_bleDev = BleDevice(&dev);
	if (!filterScanEvent(dev)) {
		return; // advertisement does not match filter
	}
	Serial.println(_bleDev.address().c_str());
	// now call the user registered callback
	void (*callback_func)(microapp_ble_dev_t) = (void (*)(microapp_ble_dev_t)) _scanned_device_callback;
	callback_func(dev);
}

bool Ble::filterScanEvent(microapp_ble_dev_t dev)
{
	switch (_activeFilter.type) {
		case BleFilterAddress: {
			// Serial.print("Scanned device MAC address "); Serial.println(dev.addr, sizeof(dev.addr));
			if (memcmp(dev.addr,_activeFilter.address.byte,MAC_ADDRESS_LENGTH) != 0) return false;
			break;
		}
		case BleFilterLocalName: {
			data_ptr_t cln; // complete local name
			data_ptr_t sln; // shortened local name
			// check if either cln or sln can be found in ad data
			bool isCLN = findAdvType(GapAdvType::CompleteLocalName,dev.data,dev.dlen,&cln);
			bool isSLN = findAdvType(GapAdvType::ShortenedLocalName,dev.data,dev.dlen,&sln);
			data_ptr_t* ln;
			if (isCLN) {
				ln = &cln;
			}
			else if (isSLN) {
				ln = &sln;
			}
			else {
				return false;
			}
			char * deviceName = (char*) ln->data;
			if (ln->len != _activeFilter.len) return false; // device name and filter name don't have same length
			if (memcmp(deviceName,_activeFilter.name,ln->len) != 0) return false; // local name doesn't match filter name
			break;
		}
		case BleFilterUuid: {
			// TODO: refactor. Now filters ads of type service data with as first element the filtered uuid
			data_ptr_t serviceData;
			if (!findAdvType(GapAdvType::ServiceData, dev.data, dev.dlen,&serviceData)) return false;
			uint16_t uuid = ((serviceData.data[1] << 8) | serviceData.data[0]);
			if (uuid != _activeFilter.uuid) return false; // service data uuid does not match filter uuid
			break;
		}
		case BleFilterNone:
		default:
			break;
	}
	return true;
}

void Ble::setEventHandler(BleEventType type, void (*isr)(microapp_ble_dev_t))
{
	Serial.println("Setting event handler");
	// TODO: do something with type. For now assume type is BleEventDeviceScanned
	microapp_ble_cmd_t *ble_cmd = (microapp_ble_cmd_t*)&global_msg;
	ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
	ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;
	ble_cmd->callback = (uintptr_t)(handleScanEventWrapper);

	global_msg.length = sizeof(microapp_ble_cmd_t);

	sendMessage(&global_msg);

	_scanned_device_callback = (uintptr_t)(isr);
}

bool Ble::scan(bool withDuplicates)
{
	if (_isScanning) return true; // already scanning

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

bool Ble::scanForName(const char* name, bool withDuplicates)
{
	_activeFilter.type = BleFilterLocalName;
	_activeFilter.len = strlen(name);
	strcpy(_activeFilter.name, name);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForAddress(const char* address, bool withDuplicates)
{
	_activeFilter.type = BleFilterAddress;
	_activeFilter.address = convertStringToMac(address);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

bool Ble::scanForUuid(const char* uuid, bool withDuplicates)
{
	_activeFilter.type = BleFilterUuid;
	_activeFilter.uuid = convertStringToUuid(uuid);
	// TODO: do something with withDuplicates argument
	return scan(withDuplicates);
}

void Ble::stopScan()
{
	if (!_isScanning) return; // already not scanning

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

BleFilter* Ble::getFilter()
{
	return &_activeFilter;
}
