#pragma once

#include <BleDevice.h>
#include <BleService.h>
#include <BleUtils.h>
#include <Serial.h>
#include <microapp.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <microapp.h>

#ifdef __cplusplus
}
#endif

// Types of BLE event for which event handlers can be set
// The naming of these corresponds with ArduinoBLE syntax
enum BleEventType {
	BLEDeviceScanned = 0x01,
	BLEConnected     = 0x02,
	BLEDisconnected  = 0x03,
	BLESubscribed    = 0x04,
	BLEUnsubscribed  = 0x05,
	BLERead          = 0x06,
	BLEWritten       = 0x07,
};

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
	char name[MAX_BLE_ADV_DATA_LENGTH];  // max length of name equals max advertisement length
	uint16_t len;                        // length of the name field
	uuid16_t uuid;                       // service data uuid
};

typedef void (*BleEventHandler)(BleDevice);

// Context for the callback that can be kept local.
struct BleInterruptContext {
	BleEventHandler eventHandler = nullptr;
	bool filled                  = false;
	MicroappSdkBleType type      = CS_MICROAPP_SDK_BLE_NONE;
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

	BleDevice _activeDevice;

	BleFilter _activeFilter;

	bool _isScanning = false;

	static const uint8_t MAX_BLE_INTERRUPT_REGISTRATIONS = 3;

	/*
	 * Store callback contexts.
	 */
	BleInterruptContext _bleInterruptContext[MAX_BLE_INTERRUPT_REGISTRATIONS];

	/**
	 * Compares the scanned device device against the filter and returns true upon a match
	 *
	 * @param[in] rawDevice the scanned BLE device
	 *
	 */
	bool filterScanEvent(BleDevice rawDevice);

	/**
	 * Get the currently set filter for scanned devices
	 *
	 * @return A pointer to the BleFilter object
	 */
	BleFilter* getFilter();

	/**
	 * Handles interrupts entering the BLE class
	 */
	microapp_sdk_result_t handleInterrupt(microapp_sdk_ble_t* ble);

	/**
	 * Converts a (user-facing) event type to the corresponding MicroappSdkBleType
	 */
	MicroappSdkBleType getBleType(BleEventType eventType);

	microapp_sdk_result_t setInterruptContext(BleEventType eventType, void (*eventHandler)(BleDevice));

	microapp_sdk_result_t getInterruptContext(BleEventType eventType, BleInterruptContext& context);

	microapp_sdk_result_t removeInterruptContext(BleEventType eventType);

public:
	static Ble& getInstance() {
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Initializes the BLE module
	 *
	 * @return true on success
	 * @return false otherwise
	 */
	bool begin();

	/**
	 * Stops the BLE module
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
	 * Disconnect any connected BLE device
	 *
	 * @return true on success
	 * @return false otherwise
	 */
	bool disconnect();

	/**
	 * Get the MAC address of the own device
	 *
	 * @return String representation of the MAC address
	 */
	String address();

	/**
	 * Get the RSSI of the connected BLE device
	 *
	 * @return RSSI of connected device. 127 if not BLE device is connected
	 */
	int8_t rssi();

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
	BleDevice available();

	/**
	 * Add a BLEService to the set of services the BLE device provides
	 *
	 * @param service BLEService to add
	 */
	void addService(BleService service);

	/**
	 * Query the central BLE device connected
	 *
	 * @return BleDevice representing the central
	 */
	BleDevice central();

	/**
	 * Set if the device is connectable after advertising, defaults to true
	 *
	 * @param connectable if true the device will be connectable, if false not connectable
	 */
	void setConnectable(bool connectable);

};

#define BLE Ble::getInstance()
