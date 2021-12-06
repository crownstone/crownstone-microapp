#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	microapp_ble_dev_t* _dev; // pointer to the raw advertisement data

	char _address[MAC_ADDRESS_STRING_LENGTH+1]; // 17 chars for address plus 1 escape char

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31)

	bool _hasCLN; // has a complete local name field

	bool _hasSLN; // has a shortened local name field

public:

	BleDevice(microapp_ble_dev_t* dev = nullptr) : _dev(dev) {};

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
