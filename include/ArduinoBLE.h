#pragma once

#include <BleUtils.h>
#include <BleService.h>
#include <BleScan.h>
#include <BleDevice.h>
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
	BleFilterType type;  // defines which property is currently being filtered on
	MacAddress address;
	char localName[MAX_BLE_ADV_DATA_LENGTH];  // max length of name equals max advertisement length
	uint16_t localNameLen;                    // length of the name field
	Uuid uuid;                                // service data uuid
};

typedef void (*BleEventHandler)(BleDevice);

// Registration for the callback that can be kept local.
struct BleEventHandlerRegistration {
	BleEventHandler eventHandler = nullptr;
	bool filled                  = false;
	BleEventType eventType;
};

/**
 * Main class for scanning, connecting and handling Bluetooth Low Energy devices
 *
 * Singleton class which can be called by the user via the macro BLE.
 */
class Ble {
private:
	friend microapp_sdk_result_t handleBleInterrupt(void*);

	Ble(){};

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool initialized : 1;   // begin has been called
			bool isScanning : 1;    // scans are handled
		} flags;
		uint8_t asInt = 0;  // initialize to zero
	} _flags;

	MacAddress _address; // address of the crownstone

	// Device only used for incoming scans
	// Is overwritten as new scans come in that pass the filter
	BleDevice _scanDevice;
	// Filter for device scans
	BleFilter _scanFilter;
	// Main device acting as either central or peripheral
	BleDevice _device;

	// References to local services are stored here.
	// The actual services and their characteristics are stored on the user side
	static const uint8_t MAX_LOCAL_SERVICES = 2;
	BleService* _services[MAX_LOCAL_SERVICES]; // array of pointers
	uint8_t _serviceCount = 0;

	// Discovered remote services, characteristics and their values are stored here
	static const uint8_t MAX_REMOTE_SERVICES = 2;
	BleService _remoteServices[MAX_REMOTE_SERVICES];
	uint8_t _remoteServiceCount = 0;
	static const uint8_t MAX_REMOTE_CHARACTERISTICS = 10;
	BleCharacteristic _remoteCharacteristics[MAX_REMOTE_CHARACTERISTICS];
	uint8_t _remoteCharacteristicCount = 0;
	static const uint8_t MAX_REMOTE_VALUE_SIZE = 20;
	struct remote_value_t {
		uint8_t buffer[MAX_REMOTE_VALUE_SIZE];
	};
	remote_value_t _remoteValues[MAX_REMOTE_CHARACTERISTICS];

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
	 */
	microapp_sdk_result_t handleEvent(microapp_sdk_ble_t* ble);

	microapp_sdk_result_t handleScanEvent(microapp_sdk_ble_scan_t* scan);
	microapp_sdk_result_t handleCentralEvent(microapp_sdk_ble_central_t* central);
	microapp_sdk_result_t handlePeripheralEvent(microapp_sdk_ble_peripheral_t* peripheral);

	/**
	 * Get a characteristic based on its handle
	 *
	 * @param[in] handle the handle of the characteristic
	 * @param[out] characteristic if found, reference to characteristic will be placed here
	 * @return true if characteristic found
	 * @return false otherwise
	 */
	microapp_sdk_result_t getCharacteristic(uint16_t handle, BleCharacteristic& characteristic);

	/**
	 * Register interrupts for event of a specific bleType
	 *
	 * @param bleType the bleType of the ble sdk message
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS if successful, otherwise error code
	 */
	microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType bleType);

	/**
	 * Locally register event handlers for a new callback set by the user
	 *
	 * @param eventType BleEventType indicating he type of event, e.g. BLEConnected
	 * @param eventHandler callback to call upon the event specified by eventType
	 * @return microapp_sdk_result_t
	 */
	microapp_sdk_result_t registerEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice));

	/**
	 * Based on the event type, get the event handler registration
	 *
	 * @param eventType the type of BLE event for which to get the registration
	 * @param registration an empty instance of BleEventHandlerRegistration in which the result is placed
	 * @return microapp_sdk_result_t
	 */
	microapp_sdk_result_t getEventHandlerRegistration(BleEventType eventType, BleEventHandlerRegistration& registration);

	/**
	 * Remove the event handler registration
	 *
	 * @param eventType the type of BLE event for which to remove the registration
	 * @return microapp_sdk_result_t
	 */
	microapp_sdk_result_t removeEventHandlerRegistration(BleEventType eventType);

public:
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
	 * @param timeout
	 */
	void poll(int timeout = 0);

	/**
	 * Registers a callback function for scanned device event triggered within bluenet
	 *
	 * @param[in] eventType   Type of event to set callback for
	 * @param[in] callback    The callback function to call upon a trigger
	 *
	 * @return                True if successful
	 */
	bool setEventHandler(BleEventType eventType, void (*callback)(BleDevice));

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
	 * @return true if any BLE device that was previously connected was disconnected
	 * @return false otherwise
	 */
	bool disconnect();

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
	 * Set if the device is connectable after advertising, defaults to true
	 *
	 * @param connectable if true the device will be connectable, if false not connectable
	 */
	void setConnectable(bool connectable);

	/**
	 * Sends command to bluenet to call registered microapp callback function upon receiving advertisements
	 *
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scan(bool withDuplicates = false);

	/**
	 * Registers filter with name name and calls scan()
	 *
	 * @param[in] name            String containing the local name to filter on, advertised as either the complete or
	 * shortened local name
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scanForName(const char* name, bool withDuplicates = false);

	/**
	 * Registers filter with MAC address address and calls scan()
	 *
	 * @param[in] address         MAC address string of the format "AA:BB:CC:DD:EE:FF" to filter on, either lowercase or
	 * uppercase letters.
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scanForAddress(const char* address, bool withDuplicates = false);

	/**
	 * Registers filter with service data uuid uuid and calls scan()
	 *
	 * @param[in] uuid            16-bit UUID string, e.g. "180D" (Heart Rate), either lowercase or uppercase letters.
	 * See https://www.bluetooth.com/specifications/assigned-numbers/
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scanForUuid(const char* uuid, bool withDuplicates = false);

	/**
	 * Sends command to bluenet to stop calling registered microapp callback function upon receiving advertisements
	 */
	bool stopScan();

	/**
	 * Returns last scanned device which matched the filter
	 *
	 * @return                  BleDevice object representing the discovered device
	 */
	BleDevice& available();
};

#define BLE Ble::getInstance()
