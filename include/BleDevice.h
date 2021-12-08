#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	microapp_ble_dev_t _device; // pointer to the raw advertisement data

	char _address[MAC_ADDRESS_STRING_LENGTH]; // 'stringified' mac address

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31 bytes)

	bool _hasCompleteLocalName; // has a complete local name field

	bool _hasShortenedLocalName; // has a shortened local name field

public:

	BleDevice(){}; // default constructor

	BleDevice(const microapp_ble_dev_t & dev) : _device(dev) {};

	/*
	 * Returns the raw advertisement data for device-specific advertisement processing
	 */
	microapp_ble_dev_t* rawData();

	/*
	 * Returns device address in the format "AA:BB:CC:DD:EE:FF"
	 */
	String address();

	/*
	 * Returns rssi value of last scanned advertisement
	 */
	int8_t rssi();

	/*
	 * Returns whether the device has advertised a local name (either complete or shortened)
	 */
	bool hasLocalName();

	/*
	 * Returns the advertised local name of the device as a string
	 */
	String localName();

};
