#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

class BleDevice {

private:

	microapp_ble_dev_t* _dev;

	char _address_str[MAC_ADDRESS_STRING_LENGTH+1];

	// friend class Ble;

public:

	BleDevice(microapp_ble_dev_t* dev = nullptr) : _dev(dev) {};

	String address();

	int8_t rssi();

	bool hasLocalName();

	String localName();

};
