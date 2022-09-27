#pragma once

#include <BleDevice.h>
#include <BleScan.h>
#include <BleService.h>
#include <BleUtils.h>
#include <BleMacAddress.h>
#include <BleUuid.h>
#include <Serial.h>
#include <microapp.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <microapp.h>

#ifdef __cplusplus
}
#endif

// Types of filters which can be used to filter scanned BLE devices
enum BleFilterType {
	BleFilterNone = 0,  // default
	BleFilterAddress,
	BleFilterLocalName,
	BleFilterUuid
};

// Stores the filter for filtering scanned BLE devices
struct BleFilter {
	// defines which property is currently being filtered on
	BleFilterType type;
	// address to be filtered on
	MacAddress address;
	// max length of name equals max advertisement length + 1 for 0 termination
	char localName[MAX_BLE_ADV_DATA_LENGTH + 1];
	// length of the name field
	uint16_t localNameLen;
	// service data uuid
	Uuid uuid;
};

/**
 * Main class for scanning, connecting and handling Bluetooth Low Energy devices
 *
 * Singleton class which can be called by the user via the macro BLE.
 */
class Ble {
private:
	friend microapp_sdk_result_t handleBleInterrupt(void*);
	friend microapp_sdk_result_t registerBleEventHandler(BleEventType, BleEventHandler);
	friend microapp_sdk_result_t getBleEventHandlerRegistration(BleEventType, BleEventHandlerRegistration&);
	friend microapp_sdk_result_t removeBleEventHandlerRegistration(BleEventType);
	friend bool registeredBleInterrupt(MicroappSdkBleType);
	friend microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType);

	Ble(){};

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			//! whether begin has been called
			bool initialized : 1;
			//! whether scans are handled
			bool isScanning : 1;
			bool registeredScanInterrupts : 1;
			bool registeredCentralInterrupts : 1;
			bool registeredPeripheralInterrupts : 1;
		} flags;
		// initialize to zero
		uint8_t asInt = 0;
	} _flags;

	// Address of the crownstone itself
	MacAddress _address;

	// Device only used for incoming scans
	// Is overwritten as new scans come in that pass the filter
	BleDevice _scanDevice;
	// Filter for device scans
	BleFilter _scanFilter;

	// Remote device acting as either central or peripheral
	BleDevice _device;

	// Pointers to local services are stored here (for peripheral role)
	// The actual services and their characteristics are stored on the user side
	// The pointers are not initialized to nullptrs. Validity should be checked via _localServiceCount
	static const uint8_t MAX_LOCAL_SERVICES = 2;
	BleService* _localServices[MAX_LOCAL_SERVICES];
	uint8_t _localServiceCount = 0;

	// Discovered remote services, characteristics and their values are stored here
	static const uint8_t MAX_REMOTE_SERVICES = 2;
	BleService _remoteServices[MAX_REMOTE_SERVICES];
	uint8_t _remoteServiceCount = 0;

	static const uint8_t MAX_REMOTE_CHARACTERISTICS = 10;
	BleCharacteristic _remoteCharacteristics[MAX_REMOTE_CHARACTERISTICS];
	uint8_t _remoteCharacteristicCount = 0;

	// Event handlers set by the user
	static const uint8_t MAX_BLE_EVENT_HANDLER_REGISTRATIONS = 3;

	/*
	 * Store callbacks set by users
	 */
	BleEventHandlerRegistration _bleEventHandlerRegistration[MAX_BLE_EVENT_HANDLER_REGISTRATIONS];

	/**
	 * Compares the scanned device device against the filter and returns true upon a match
	 *
	 * @param[in] scan the scan data
	 * @param[in] address the address of the scanned device
	 * @return true if the scan passes the filter
	 * @return false otherwise
	 */
	bool matchesFilter(BleScan scan, MacAddress address);

	/**
	 * Handles interrupts entering the BLE class from bluenet
	 *
	 * @param[in] ble the SDK packet with the incoming message from bluenet
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if ble->type has no defined event behaviour
	 * @return microap_sdk_result_t of the specific ble->type handler
	 */
	microapp_sdk_result_t handleEvent(microapp_sdk_ble_t* ble);

	/**
	 * Handles interrupts entering the BLE class from bluenet of the scan type
	 *
	 * @param[in] scan the scan packet with the incoming message from bluenet
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if scan->type has no defined event behaviour
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
	 */
	microapp_sdk_result_t handleScanEvent(microapp_sdk_ble_scan_t* scan);

	/**
	 * Handles interrupts entering the BLE class from bluenet of the central type
	 *
	 * @param[in] central the central packet with the incoming message from bluenet
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if central->type has no defined event behaviour
	 * @return CS_MICROAPP_SDK_ACK_ERR_DISABLED if other device is not a peripheral, meaning central events are disabled
	 * @return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE if discovered service or characteristic cannot be added due to space
	 * limitations
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
	 * @return microapp_sdk_result_t specifying other error within handling event
	 */
	microapp_sdk_result_t handleCentralEvent(microapp_sdk_ble_central_t* central);

	/**
	 * Handles interrupts entering the BLE class from bluenet of the peripheral type
	 *
	 * @param[in] peripheral the peripheral packet with the incoming message from bluenet
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if peripheral->type has no defined event behaviour
	 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED if event handling has not been implemented yet
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
	 * @return microapp_sdk_result_t specifying other error within handling event
	 */
	microapp_sdk_result_t handlePeripheralEvent(microapp_sdk_ble_peripheral_t* peripheral);

	/**
	 * Get a characteristic based on its handle (for peripheral role)
	 *
	 * @param[in] handle the handle of the characteristic
	 * @param[out] characteristic if found, reference to characteristic will be placed here
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BLE not initialized (BLE.begin() not called)
	 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if characteristic not found (BLE.addService() or
	 * service.addCharacteristic() not called)
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS if found
	 */
	microapp_sdk_result_t getLocalCharacteristic(uint16_t handle, BleCharacteristic& characteristic);

public:
	// Should be called via BLE macro (e.g. BLE.begin())
	static Ble& getInstance() {
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Initializes the BLE device
	 *
	 * @return true on success
	 * @return false otherwise
	 */
	bool begin();

	/**
	 * Stops the BLE device
	 */
	void end();

	/**
	 * Poll for BLE events and handle them
	 *
	 * @param timeout optional timeout in ms, to wait for event. If not specified defaults to 0 ms
	 */
	void poll(int timeout = 0);

	/**
	 * Registers a callback function for scanned device event triggered within bluenet
	 *
	 * @param[in] eventType event type (BLEDeviceScanned, BLEConnected, BLEDisconnected)
	 * @param[in] callback the callback function to call upon a trigger
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool setEventHandler(BleEventType eventType, DeviceEventHandler eventHandler);

	/**
	 * Query if another BLE device is connected
	 *
	 * @return true if another BLE device is connected
	 * @return false otherwise
	 */
	bool connected();

	/**
	 * Disconnect any BLE devices that are connected
	 *
	 * @param timeout timeout in milliseconds
	 * @return true if any BLE device that was previously connected was disconnected
	 * @return false otherwise
	 */
	bool disconnect(uint32_t timeout = 5000);

	/**
	 * Query the Bluetooth address of the BLE device
	 *
	 * @return the Bluetooth address of the BLE device as a string
	 */
	String address();

	/**
	 * Query the RSSI of the connected BLE device
	 *
	 * @return the RSSI of connected BLE device. 127 if no BLE device is connected
	 */
	int8_t rssi();

	/**
	 * Add a BLEService to the set of services the BLE device provides
	 *
	 * @param service BLEService to add
	 */
	void addService(BleService& service);

	/**
	 * Query the central BLE device connected
	 *
	 * @return BleDevice representing the central
	 */
	BleDevice& central();

	/**
	 * Sends command to bluenet to call registered microapp callback function upon receiving advertisements
	 *
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool scan(bool withDuplicates = false);

	/**
	 * Registers filter with name name and calls scan()
	 *
	 * @param[in] name            String containing the local name to filter on, advertised as either the complete or
	 * shortened local name
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool scanForName(const char* name, bool withDuplicates = false);

	/**
	 * Registers filter with MAC address address and calls scan()
	 *
	 * @param[in] address         MAC address string of the format "AA:BB:CC:DD:EE:FF" to filter on, either lowercase or
	 * uppercase letters.
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool scanForAddress(const char* address, bool withDuplicates = false);

	/**
	 * Registers filter with service data uuid uuid and calls scan()
	 *
	 * @param[in] uuid            16-bit UUID string, e.g. "180D" (Heart Rate), either lowercase or uppercase letters.
	 * See https://www.bluetooth.com/specifications/assigned-numbers/
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool scanForUuid(const char* uuid, bool withDuplicates = false);

	/**
	 * Sends command to bluenet to stop calling registered microapp callback function upon receiving advertisements
	 */
	bool stopScan();

	/**
	 * Returns last scanned device which matched the filter
	 *
	 * @return BleDevice object representing the discovered device
	 */
	BleDevice& available();
};

#define BLE Ble::getInstance()

/**
 * The following functions are outside the Ble class so they can be accessed
 * from member functions of e.g. the BleDevice class.
 * If this somehow can be done, these functions can be moved back within the Ble class
 */

/**
 * Locally register event handlers for a new callback set by the user
 *
 * @param eventType BleEventType indicating he type of event, e.g. BLEConnected
 * @param eventHandler callback to call upon the event specified by eventType
 * @return CS_MICROAPP_SDK_ACK_ERR_ALREADY_EXISTS if a handler already registered for this eventType
 * @return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE if no space left for a handler registration
 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
 */
microapp_sdk_result_t registerBleEventHandler(BleEventType eventType, BleEventHandler eventHandler);

/**
 * Based on the event type, get the event handler registration
 *
 * @param eventType the type of BLE event for which to get the registration
 * @param registration an empty instance of BleEventHandlerRegistration in which the result is placed
 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if the eventHandler field is empty
 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if no registration is found for the provided eventType
 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
 */
microapp_sdk_result_t getBleEventHandlerRegistration(BleEventType eventType, BleEventHandlerRegistration& registration);

/**
 * Remove the event handler registration
 *
 * @param eventType the type of BLE event for which to remove the registration
 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if no registration is found for the provided eventType
 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
 */
microapp_sdk_result_t removeBleEventHandlerRegistration(BleEventType eventType);

/**
 * Check if interrupts are registered for given bleType
 *
 * @param bleType the bleType of the ble sdk message
 * @return true if already registered
 * @return false otherwise
 */
bool registeredBleInterrupt(MicroappSdkBleType bleType);

/**
 * Register interrupts for event of a specific bleType
 *
 * @param bleType the bleType of the ble sdk message
 * @return CS_MICROAPP_SDK_ACK_SUCCESS upon success
 * @return microapp_sdk_result_t specifying other error types
 */
microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType bleType);
