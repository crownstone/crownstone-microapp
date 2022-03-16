#pragma once

#include <Serial.h>
#include <microapp.h>
#include <BleUtils.h>
#include <BleDevice.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <microapp.h>

// Create shortened typedefs (we are within microapp scope here)

// Types of BLE event for which event handlers can be set
typedef MicroappBleEventType BleEventType;

#ifdef __cplusplus
}
#endif

// Types of filters which can be used to filter scanned BLE devices
enum BleFilterType {
	BleFilterNone = 0, // default
	BleFilterAddress,
	BleFilterLocalName,
	BleFilterUuid
};

// Stores the filter for filtering scanned BLE devices
struct BleFilter {
	BleFilterType type; // defines which property is currently being filtered on
	MacAddress address;
	char name[MAX_BLE_ADV_DATA_LENGTH]; // max length of name equals max advertisement length
	uint16_t len; // length of the name field
	uuid16_t uuid; // service data uuid
};

typedef void (*BleEventHandler)(BleDevice);

// Context for the callback that can be kept local.
struct BleSoftInterruptContext {
	BleEventHandler eventHandler;
	bool filled;
	uint8_t id;
};

/**
 * Main class for scanning, connecting and handling Bluetooth Low Energy devices
 *
 * Singleton class which can be called by the user via the macro BLE.
 */
class Ble {
private:
	Ble();

	BleDevice _activeDevice;

	BleFilter _activeFilter;

	bool _isScanning = false;

	/*
	 * Compares the scanned device device against the filter and returns true upon a match
	 */
	bool filterScanEvent(BleDevice rawDevice);

	/*
	 * Store callback contexts.
	 */
	BleSoftInterruptContext _bleSoftInterruptContext[MAX_SOFT_INTERRUPTS];

public:

	static Ble & getInstance() {
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Registers a callback function for scanned device event triggered within bluenet
	 *
	 * @param[in] eventType   Type of event to set callback for
	 * @param[in] callback    The callback function to call upon a trigger
	 */
	void setEventHandler(BleEventType eventType, void (*callback)(BleDevice));

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
	 * @param[in] name            String containing the local name to filter on, advertised as either the complete or shortened local name
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scanForName(const char* name, bool withDuplicates = false);

	/**
	 * Registers filter with MAC address address and calls scan()
	 *
	 * @param[in] address         MAC address string of the format "AA:BB:CC:DD:EE:FF" to filter on, either lowercase or uppercase letters.
	 * @param[in] withDuplicates  If true, returns duplicate advertisements. (Not implemented)
	 *
	 * @return                    True if successful
	 */
	bool scanForAddress(const char* address, bool withDuplicates = false);

	/**
	 * Registers filter with service data uuid uuid and calls scan()
	 *
	 * @param[in] uuid            16-bit UUID string, e.g. "180D" (Heart Rate), either lowercase or uppercase letters. See https://www.bluetooth.com/specifications/assigned-numbers/
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

protected:
	/**
	 * Get the currently set filter for scanned devices
	 *
	 * @return                 A pointer to the BleFilter object
	 */
	BleFilter* getFilter();

};


#define BLE Ble::getInstance()
