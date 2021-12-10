#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	friend class Ble; // allows only the BLE class to construct BleDevice objects

	BleDevice(){}; // default constructor

	BleDevice(const microapp_ble_device_t & dev) : _device(dev) {_flags.flags.nonEmpty = true;}; // non-empty constructor

	microapp_ble_device_t _device; // the raw advertisement data

	char _address[MAC_ADDRESS_STRING_LENGTH]; // 'stringified' mac address
	uint8_t _addressLen = 0;

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31 bytes)
	uint8_t _localNameLen = 0;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool nonEmpty              : 1; // device is not empty
			bool hasCompleteLocalName  : 1; // has a complete local name field
			bool hasShortenedLocalName : 1; // has a shortened local name field
			bool checkedLocalName      : 1; // _device has already been checked for local name field
			bool cachedLocalName       : 1; // local name is cached in _localName
			bool cachedAddress         : 1; // address is cached in _address
		} flags;
		uint8_t asInt = 0; // initialize to zero
	} _flags;

public:
	// return true if BleDevice is nontrivial, i.e. initialized from an actual advertisement
	explicit operator bool() const {return _flags.flags.nonEmpty;}

	/**
	 * Get the raw advertisement data for device-specific advertisement processing.
	 *
	 * @return        A pointer to the microapp_ble_device_t object.
	 */
	microapp_ble_device_t* rawData();

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
	 * @return      String of the advertisement data in either the complete local name field or the shortened local name field.
	 *              Returns empty string if the device does not advertise a local name.
	 */
	String localName();

};
