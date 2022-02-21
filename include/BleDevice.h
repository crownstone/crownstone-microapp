#pragma once

#include <microapp.h>
#include <String.h>
#include <BleUtils.h>

// struct containing a pointer to a block of data, and a length field to indicate the length of the block
struct data_ptr_t {
	uint8_t* data = nullptr;
	size_t len = 0;
};

// GAP advertisement types, see
// https://www.bluetooth.com/specifications/assigned-numbers/
enum GapAdvType {
	CompleteList16BitServiceUuids  = 0x03,
	CompleteList128BitServiceUuids = 0x07,
	ShortenedLocalName             = 0x08,
	CompleteLocalName              = 0x09,
	ServiceData                    = 0x16,
	ManufacturerSpecificData       = 0xFF
};

class BleDevice {

private:

	friend class Ble; // exceptions for Ble class

	BleDevice(){}; // default constructor

	microapp_ble_device_t _device; // the raw advertisement data

	MacAddress _address;

	char _localName[MAX_BLE_ADV_DATA_LENGTH]; // maximum length equals max ble advertisement length (31 bytes)
	uint8_t _localNameLen = 0;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool nonEmpty              : 1; // device is not empty
			bool hasCompleteLocalName  : 1; // has a complete local name field
			bool hasShortenedLocalName : 1; // has a shortened local name field
			bool checkedLocalName      : 1; // _device has already been checked for local name field
			bool cachedLocalName       : 1; // local name is cached in _localName
			bool connected             : 1; // whether peripheral device is connected
		} flags;
		uint8_t asInt = 0; // initialize to zero
	} _flags;

public:

	BleDevice(const microapp_ble_device_t & dev); // non-empty constructor

	// return true if BleDevice is nontrivial, i.e. initialized from an actual advertisement
	explicit operator bool() const {return _flags.flags.nonEmpty;}

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

	bool connect();
	bool connected();

	/**
	 * Tries to find an ad of specified GAP ad data type. and if found returns true and a pointer to its location.
	 *
	 * @param[in] type          GAP advertisement type according to https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile.
	 * @param[out] foundData    data_ptr_t containing a pointer to the first byte of advData containing the data of type type and its length.
	 *
	 * @return true             if the advertisement data of given type is found.
	 * @return false            if the advertisement data of given type is not found.
	 */
	bool findAdvertisementDataType(GapAdvType type, data_ptr_t* foundData);


};
