#include <Arduino.h>
#include <BleDevice.h>

// Construct as a peripheral device
BleDevice::BleDevice(BleScan scan, MacAddress address, rssi_t rssi) {
	memcpy(_scanData, scan._scanData, scan._scanSize);
	_scan                     = BleScan(_scanData, scan._scanSize);
	_address                  = address;
	_rssi                     = rssi;
	_flags.flags.isPeripheral = true;
	_flags.flags.initialized  = true;
}

// Construct as a central device
BleDevice::BleDevice(MacAddress address) {
	_address                 = address;
	_flags.flags.isCentral   = true;
	_flags.flags.initialized = true;
}

BleDevice::operator bool() const {
	return _flags.flags.initialized;
}

// Defined for both central and peripheral devices
void BleDevice::onConnect(uint16_t connectionHandle) {
	_flags.flags.connected = true;
	_connectionHandle = connectionHandle;
	// set async result flag
	if (_flags.flags.isPeripheral) {
		_asyncResult = BleAsyncSuccess;
	}
}

// Defined for both central and peripheral devices
void BleDevice::onDisconnect() {
	// clear connection related variables and flags
	_connectionHandle = 0;
	_serviceCount = 0;
	_flags.flags.connected = false;
	_flags.flags.discoveryDone = false;
	// set async result flag
	if (_flags.flags.isPeripheral) {
		_asyncResult = BleAsyncSuccess;
	}
}

void BleDevice::onDiscoverDone() {
	_flags.flags.discoveryDone = true;
	// set async result flag
	if (_flags.flags.isPeripheral) {
		_asyncResult = BleAsyncSuccess;
	}
}

// Only defined for peripheral devices
microapp_sdk_result_t BleDevice::addDiscoveredService(BleService* service) {
	if (_serviceCount >= MAX_REMOTE_SERVICES) {
		return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
	}
	_services[_serviceCount] = service;
	_serviceCount++;
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

// Only defined for peripheral devices
microapp_sdk_result_t BleDevice::addDiscoveredCharacteristic(BleCharacteristic* characteristic, Uuid serviceUuid) {
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->_uuid == serviceUuid) {
			return _services[i]->addDiscoveredCharacteristic(characteristic);
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

// Only defined for peripheral devices
microapp_sdk_result_t BleDevice::getCharacteristic(uint16_t handle, BleCharacteristic** characteristic) {
	if (!_flags.flags.initialized) {
		return CS_MICROAPP_SDK_ACK_ERR_EMPTY;
	}
	if (!_flags.flags.isPeripheral) {
		return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		microapp_sdk_result_t result = _services[i]->getCharacteristic(handle, characteristic);
		if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
			return result;
		}
	}
	return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND;
}

bool BleDevice::waitForAsyncResult(uint8_t timeout) {
	// Before calling this function, the asyncResult variable needs
	// to be set to BleAsyncWaiting. It will even need to be set before
	// the sendMessage call with the request to bluenet
	uint8_t tries = timeout / MICROAPP_LOOP_INTERVAL_MS;
	while (_asyncResult == BleAsyncWaiting) {
		// Yield. Upon an event from bluenet asyncResult will be set
		delay(MICROAPP_LOOP_INTERVAL_MS);
		if (--tries == 0) {
			return false;
		}
	}
	if (_asyncResult == BleAsyncFailure) {
		_asyncResult = BleAsyncNotWaiting;
		return false;
	}
	_asyncResult = BleAsyncNotWaiting;
	return true;
}

// Defined for both central and peripheral devices
void BleDevice::poll(uint32_t timeout) {
	if (timeout == 0) {
		return;
	}
	delay(timeout);
}

// Defined for both central and peripheral devices
bool BleDevice::connected() {
	return _flags.flags.connected;
}

// Defined for both central and peripheral devices
bool BleDevice::disconnect(uint32_t timeout) {
	// this is a blocking function
	if (!_flags.flags.connected) {
		return false;
	}
	// Indicate we are waiting for an async event with a result
	// This has to be set before the sendMessage call
	_asyncResult = BleAsyncWaiting;

	uint8_t* payload               = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	if (_flags.flags.isCentral) {
		bleRequest->type                     = CS_MICROAPP_SDK_BLE_CENTRAL;
		bleRequest->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_DISCONNECT;
		bleRequest->central.connectionHandle = _connectionHandle;
	}
	else if (_flags.flags.isPeripheral) {
		bleRequest->type                        = CS_MICROAPP_SDK_BLE_PERIPHERAL;
		bleRequest->peripheral.type             = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_DISCONNECT;
		bleRequest->peripheral.connectionHandle = _connectionHandle;
	}
	sendMessage();
	microapp_sdk_result_t result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return true;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return false;
	}
	return waitForAsyncResult(timeout);
}

// Defined for both central and peripheral devices
String BleDevice::address() {
	return String(_address.string());
}

// Only defined for peripheral devices
int8_t BleDevice::rssi() {
	return _rssi;
}

// Only defined for peripheral devices
bool BleDevice::discoverAttributes() {
	// Not implemented!
	// Bluenet requires a set of services to initiate discovery
	// Hence, discovery is currently only possible via discoverService
	return false;
}

// Only defined for peripheral devices
bool BleDevice::discoverService(const char* serviceUuid, uint32_t timeout) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	if (_flags.flags.discoveryDone) {
		return true;
	}
	microapp_sdk_result_t result;
	Uuid uuid(serviceUuid);
	if (!uuid.registered()) {
		result = uuid.registerCustom();
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return false;
		}
	}
	// Indicate we are waiting for an async event with a result
	// This has to be set before the sendMessage call
	_asyncResult = BleAsyncWaiting;

	uint8_t* payload                                  = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest                    = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType                    = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                            = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                                  = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type                          = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_DISCOVER;
	bleRequest->central.requestDiscover.uuidCount     = 1;
	bleRequest->central.requestDiscover.uuids[0].type = uuid.getType();
	bleRequest->central.requestDiscover.uuids[0].uuid = uuid.uuid16();
	bleRequest->central.connectionHandle              = _connectionHandle;

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return true;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		// direct failure
		return false;
	}
	return waitForAsyncResult(timeout);
}

// Only defined for peripheral devices
uint8_t BleDevice::serviceCount() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return 0;
	}
	if (!_flags.flags.discoveryDone) {
		return 0;
	}
	return _serviceCount;
}

// Only defined for peripheral devices
bool BleDevice::hasService(const char* serviceUuid) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	if (!_flags.flags.discoveryDone) {
		return false;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->_uuid == Uuid(serviceUuid)) {
			return true;
		}
	}
	return false;
}

// Only defined for peripheral devices
BleService& BleDevice::service(const char* uuid) {
	static BleService empty = BleService();
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return empty;
	}
	if (!_flags.flags.discoveryDone) {
		return empty;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->_uuid == Uuid(uuid)) {
			return *_services[i];
		}
	}
	return empty;
}

// Only defined for peripheral devices
uint8_t BleDevice::characteristicCount() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return 0;
	}
	if (!_flags.flags.discoveryDone) {
		return 0;
	}
	uint8_t characteristicCount = 0;
	for (uint8_t i = 0; i < _serviceCount; i++) {
		characteristicCount += _services[i]->characteristicCount();
	}
	return characteristicCount;
}

// Only defined for peripheral devices
bool BleDevice::hasCharacteristic(const char* uuid) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	if (!_flags.flags.discoveryDone) {
		return false;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->hasCharacteristic(uuid)) {
			return true;
		}
	}
	return false;
}

// Only defined for peripheral devices
BleCharacteristic& BleDevice::characteristic(const char* uuid) {
	static BleCharacteristic empty;
	empty = BleCharacteristic();
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return empty;
	}
	if (!_flags.flags.discoveryDone) {
		return empty;
	}
	for (uint8_t i = 0; i < _serviceCount; i++) {
		if (_services[i]->hasCharacteristic(uuid)) {
			return _services[i]->characteristic(uuid);
		}
	}
	return empty;
}

// Only defined for peripheral devices
bool BleDevice::hasLocalName() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	return (_scan.localName().len != 0);
}

// Only defined for peripheral devices
String BleDevice::localName() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return String(nullptr);
	}
	ble_ad_t localName = _scan.localName();
	if (localName.len == 0) {
		return String(nullptr);
	}
	static char localNameString[MAX_BLE_ADV_DATA_LENGTH + 1];
	memcpy(localNameString, localName.data, localName.len);
	localNameString[localName.len] = 0;
	return String(localNameString);
}

bool BleDevice::hasAdvertisedServiceUuid() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	return _scan.hasServiceUuid();
}

uint8_t BleDevice::advertisedServiceUuidCount() {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	return _scan.serviceUuidCount();
}

String BleDevice::advertisedServiceUuid(uint8_t index) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return String(nullptr);
	}
	Uuid uuid = Uuid(_scan.serviceUuid(index), CS_MICROAPP_SDK_BLE_UUID_STANDARD);
	return String(uuid.string());
}

// Only defined for peripheral devices
bool BleDevice::connect(uint32_t timeout) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	if (_flags.flags.connected) {
		// already connected
		return true;
	}
	microapp_sdk_result_t result;
	// First register interrupts
	if (!registeredBleInterrupt(CS_MICROAPP_SDK_BLE_CENTRAL)) {
		result = registerBleInterrupt(CS_MICROAPP_SDK_BLE_CENTRAL);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			return false;
		}
	}
	// Indicate we are waiting for an async event with a result
	// This has to be set before the sendMessage call
	_asyncResult = BleAsyncWaiting;

	// Next, request connect
	uint8_t* payload                                = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest                  = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType                  = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                          = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                                = CS_MICROAPP_SDK_BLE_CENTRAL;
	bleRequest->central.type                        = CS_MICROAPP_SDK_BLE_CENTRAL_REQUEST_CONNECT;
	bleRequest->central.connectionHandle            = _connectionHandle;
	bleRequest->central.requestConnect.address.type = _address.type();
	memcpy(bleRequest->central.requestConnect.address.address, _address.bytes(), MAC_ADDRESS_LENGTH);

	sendMessage();
	result = (microapp_sdk_result_t)bleRequest->header.ack;
	if (result == CS_MICROAPP_SDK_ACK_SUCCESS) {
		// direct success
		return true;
	}
	if (result != CS_MICROAPP_SDK_ACK_IN_PROGRESS) {
		return false;
	}
	return waitForAsyncResult(timeout);
}

// Only defined for peripheral devices
bool BleDevice::findAdvertisementDataType(GapAdvType type, ble_ad_t* foundData) {
	if (!_flags.flags.initialized || !_flags.flags.isPeripheral) {
		return false;
	}
	return _scan.findAdvertisementDataType(type, foundData);
}

// Only defined for central devices
void BleDevice::connectionKeepAlive() {
	if (!_flags.flags.initialized || !_flags.flags.isCentral) {
		return;
	}
	uint8_t* payload                                = getOutgoingMessagePayload();
	microapp_sdk_ble_t* bleRequest                  = (microapp_sdk_ble_t*)(payload);
	bleRequest->header.messageType                  = CS_MICROAPP_SDK_TYPE_BLE;
	bleRequest->header.ack                          = CS_MICROAPP_SDK_ACK_REQUEST;
	bleRequest->type                                = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	bleRequest->peripheral.type                     = CS_MICROAPP_SDK_BLE_PERIPHERAL_REQUEST_CONNECTION_ALIVE;
	bleRequest->peripheral.connectionHandle         = _connectionHandle;

	sendMessage();
}