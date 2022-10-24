#pragma once

#include <BleUuid.h>
#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

/**
 * Struct representing BLE ad. Note that it contains a pointer to data, not the data itself
 * @param type indicates the GAP ad type, see GAPAdvType enum
 * @param len length of the ad, including this len field but excluding type field
 * @param data pointer to ad data which has valid data of size (len-1)
 */
struct ble_ad_t {
	uint8_t type        = 0;
	uint8_t len         = 0;
	const uint8_t* data = nullptr;
};

/**
 * Helper wrapper class for scan data
 * Does not contain the actual data but only a pointer to it
 */
class BleScan {
private:
	friend class BleDevice;
	friend class Ble;

	BleScan(){}; // default constructor

	/**
	 * Tries to find an ad of specified GAP ad data type in the raw scan data
	 * If found returns true and a ble_ad_t with ad type, length and pointer to its data
	 *
	 * @param[in] type          GAP advertisement type
	 * @param[out] foundData    ad containing a pointer to data and its length
	 *
	 * @return true             if the advertisement data of given type is found.
	 * @return false            if the advertisement data of given type is not found.
	 */
	static bool findAdvertisementDataType(const uint8_t* scanData, uint8_t scanSize, GapAdvType type, ble_ad_t* foundData);

public:

	/**
	 * Query the local name advertised in the scan (either complete or shortened)
	 *
	 * @return An ad with a pointer to the local name and its length (nullptr and 0 if not found, respectively)
	 */
	static ble_ad_t localName(const uint8_t* scanData, uint8_t scanSize);

	/**
	 * Checks if a service with passed uuid is advertised by the device
	 *
	 * @param uuid (optional) uuid to check. Default 0 means check for any
	 * @return true if found
	 * @return false if not found
	 */
	static bool hasServiceUuid(const uint8_t* scanData, uint8_t scanSize, uuid16_t uuid = 0);

	/**
	 * Finds the number of 16-bit service uuids advertised by the device
	 *
	 * @return the number of services advertised
	 */
	static uint8_t serviceUuidCount(const uint8_t* scanData, uint8_t scanSize);

	/**
	 * Return the uuid advertised by the device, indexed by the index parameter
	 *
	 * @param index (optional) index of the service uuid. Default 0
	 * @return uuid indexed by index. Returns 0 if not found
	 */
	static uuid16_t serviceUuid(const uint8_t* scanData, uint8_t scanSize, uint8_t index = 0);

};
