#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	microapp_ble_dev_t _device; // pointer to the raw advertisement data

	char _address[MAC_ADDRESS_STRING_LENGTH]; // 'stringified' mac address
	uint8_t _addressLen = 0;

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31 bytes)
	uint8_t _localNameLen = 0;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool hasCompleteLocalName  : 1; // has a complete local name field
			bool hasShortenedLocalName : 1; // has a shortened local name field
			bool checkedLocalName      : 1; // _device has already been checked for local name field
			bool cachedLocalName       : 1; // local name is cached in _localName
			bool cachedAddress         : 1; // address is cached in _address
		} flags;
		uint8_t asInt = 0; // initialize to zero
	} _flags;

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
