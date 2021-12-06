#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	microapp_ble_dev_t* _dev;

	char _address[MAC_ADDRESS_STRING_LENGTH+1]; // 17 chars for address plus 1 escape char

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31)

	bool _hasCLN; // has a complete local name field

	bool _hasSLN; // has a shortened local name field

public:

	BleDevice(microapp_ble_dev_t* dev = nullptr) : _dev(dev) {};

	microapp_ble_dev_t* rawData();

	String address();

	int8_t rssi();

	bool hasLocalName(); // TODO: implement

	String localName(); // TODO: implement

};
