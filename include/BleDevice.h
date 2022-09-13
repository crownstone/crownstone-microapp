#pragma once

#include <BleUtils.h>
#include <BleScan.h>
#include <String.h>
#include <microapp.h>

class BleDevice {

private:
	friend class Ble;  // exceptions for Ble class

	BleDevice(){};  // default constructor

	uint8_t _scanData[MAX_BLE_ADV_DATA_LENGTH];  // raw scan data
	BleScan _scan;                               // wrapper class pointing to _scanData
	MacAddress _address;
	rssi_t _rssi;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool initialized : 1;   // device is initialized with nondefault constructor
			bool connected : 1;     // whether device is connected
			bool isCentral : 1;     // device has central role
			bool isPeripheral : 1;  // device has peripheral role
		} flags;
		uint8_t asInt = 0;  // initialize to zero
	} _flags;

public:
	BleDevice(BleScan scan, MacAddress address, rssi_t rssi);

	// return true if BleDevice is nontrivial, i.e. initialized from an actual advertisement
	explicit operator bool() const { return _flags.flags.initialized; }

	/**
	 * Poll for BLE events and handle them
	 *
	 * @param timeout
	 */
	void poll(int timeout = 0);

	/**
	 * Query if a BLE device is connected
	 *
	 * @return true if the BLE device is connected
	 * @return false otherwise
	 */
	bool connected();

	/**
	 * Disconnect the BLE device, if connected
	 *
	 * @return true if the BLE device was disconnected
	 * @return false otherwise
	 */
	bool disconnect();

	/**
	 * Get device address of the last scanned advertisement which matched the filter.
	 *
	 * @return        String in the format "AA:BB:CC:DD:EE:FF".
	 */
	String address();

	/**
	 * Get received signal strength of last scanned advertisement of the device.
	 *
	 * @return       RSSI value of the last scanned advertisement which matched the filter.
	 */
	int8_t rssi();

	/**
	 * Returns whether the device has advertised a local name
	 *
	 * @return true    if the advertisement contains a local name field (either complete or shortened).
	 * @return false   if the advertisement does not contain a local name field.
	 */
	bool hasLocalName();

	/**
	 * Returns the advertised local name of the device as a string
	 *
	 * @return      String of the advertisement data in either the complete local name field or the shortened local name
	 * field. Returns empty string if the device does not advertise a local name.
	 */
	String localName();

	/**
	 * Connect to a BLE device
	 *
	 * @return true if the connection was successful
	 * @return false otherwise
	 */
	bool connect();

	/**
	 * Find an advertisement of type type in the scanned advertisement data
	 *
	 * @param[in] type the type of ad to look for
	 * @param[out] foundData pointer to found ad + length of the ad, if present
	 * @return true if the advertisement contains an ad of type type
	 * @return false otherwise
	 */
	bool findAdvertisementDataType(GapAdvType type, ble_ad_t* foundData);
};
