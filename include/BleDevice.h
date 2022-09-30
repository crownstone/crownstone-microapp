#pragma once

#include <BleScan.h>
#include <BleService.h>
#include <BleMacAddress.h>
#include <BleUuid.h>
#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

// Forward declarations
bool registeredBleInterrupt(MicroappSdkBleType bleType);
microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType bleType);

class BleDevice {

private:
	// exceptions for Ble related classes
	friend class Ble;

	// private empty constructor
	BleDevice(){};

	// constructor from scan (peripheral)
	BleDevice(BleScan scan, MacAddress address, rssi_t rssi);
	// constructor from connect (central)
	BleDevice(MacAddress address);

	// raw scan data
	uint8_t _scanData[MAX_BLE_ADV_DATA_LENGTH];
	// wrapper class pointing to _scanData
	BleScan _scan;

	MacAddress _address;
	rssi_t _rssi = 127;

	// connectionHandle used internally for communication with bluenet
	uint16_t _connectionHandle = 0;

	// Services with characteristics for peripheral devices
	static const uint8_t MAX_SERVICES = 2;
	BleService* _services[MAX_SERVICES];  // array of pointers
	uint8_t _serviceCount = 0;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			// device is initialized with nondefault constructor
			bool initialized : 1;
			// whether device is connected
			bool connected : 1;
			// device has central role
			bool isCentral : 1;
			// device has peripheral role
			bool isPeripheral : 1;
			// discovery has been completed (only for peripheral device)
			bool discoveryDone : 1;
		} flags;
		uint8_t asInt = 0;  // initialize to zero
	} _flags;

	/**
	 * Sets internal connected flag
	 */
	void onConnect(uint16_t connectionHandle);

	/**
	 * Clear internal connected flag
	 */
	void onDisconnect();

	/**
	 * Internally add a discovered service (for peripheral devices)
	 *
	 * @param service pointer to a discovered service
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE if no space for new services
	 */
	microapp_sdk_result_t addDiscoveredService(BleService* service);

	/**
	 * Internally add a discovered characteristic (for peripheral devices)
	 *
	 * @param characteristic pointer to a discovered characteristic
	 * @param serviceUuid uuid of the service to which the characteristic belongs
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if service no service with the serviceUuid was found
	 * @return microapp_sdk_result_t specifying error
	 */
	microapp_sdk_result_t addDiscoveredCharacteristic(BleCharacteristic* characteristic, Uuid serviceUuid);

	/**
	 * Get a characteristic based on its handle (for peripheral devices)
	 *
	 * @param[in] handle the handle of the characteristic
	 * @param[out] characteristic if found, pointer to characteristic pointer will be placed here
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if device is not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if device does not perform peripheral role
	 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if characteristic was not found in services
	 */
	microapp_sdk_result_t getCharacteristic(uint16_t handle, BleCharacteristic** characteristic);

public:
	// return true if BleDevice is nontrivial, i.e. initialized from an actual advertisement
	explicit operator bool() const;

	/**
	 * Poll for BLE events and handle them
	 *
	 * @param timeout in milliseconds
	 */
	void poll(uint32_t timeout = 0);

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
	 * @param timeout in milliseconds
	 * @return true if the BLE device was disconnected
	 * @return false otherwise
	 */
	bool disconnect(uint32_t timeout = 5000);

	/**
	 * Get device address of the last scanned advertisement which matched the filter.
	 *
	 * @return string in the format "AA:BB:CC:DD:EE:FF".
	 */
	String address();

	/**
	 * Get received signal strength of last scanned advertisement of the device.
	 *
	 * @return RSSI value of the last scanned advertisement which matched the filter.
	 */
	int8_t rssi();

	/**
	 * (Not implemented!) Discover all the attributes of the BLE device
	 *
	 * @return true if successful
	 * @return false on failure
	 */
	bool discoverAttributes();

	/**
	 * Discover the attributes of a particular service on the BLE device
	 *
	 * @param serviceUuid string containing uuid of the service to be discovered
	 * @param timeout in milliseconds
	 * @return true if successful
	 * @return false on failure
	 */
	bool discoverService(const char* serviceUuid, uint32_t timeout = 5000);

	/**
	 * Query the numer of services discovered for the BLE device
	 *
	 * @return the number of services discovered for the BLE device
	 */
	uint8_t serviceCount();

	/**
	 * Query if the BLE device has a particular service
	 *
	 * @return true if the device provides the service
	 * @return false otherwise
	 */
	bool hasService(const char* serviceUuid);

	/**
	 * Get a BleService representing a BLE service the device provides
	 *
	 * @param[in] uuid a string with the uuid of the characteristic to look for
	 * @return a reference (!) to the BleService with the provided uuid, if found
	 */
	BleService& service(const char* uuid);

	/**
	 * Query the numer of characteristics discovered for the BLE device
	 *
	 * @return the number of characteristics discovered for the BLE device
	 */
	uint8_t characteristicCount();

	/**
	 * Query if the BLE device has a particular characteristic
	 *
	 * @return true if the device provides the characteristic
	 * @return false otherwise
	 */
	bool hasCharacteristic(const char* uuid);

	/**
	 * Get a BleCharacteristic representing a BLE characteristic the device provides
	 *
	 * @param[in] uuid a string with the uuid of the characteristic to look for
	 * @return a reference (!) to the BleCharacteristic with the provided uuid, if found
	 */
	BleCharacteristic& characteristic(const char* uuid);

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
	 * @return string of the advertisement data in either the complete local name field or the shortened local name
	 * field Returns empty string if the device does not advertise a local name.
	 */
	String localName();

	/**
	 * Query if a discovered BLE device is advertising a service UUID
	 *
	 * @return true if the device is advertising a service UUID
	 * @return false otherwise
	 */
	bool hasAdvertisedServiceUuid();

	/**
	 * Query the number of advertised services a discovered BLE device is advertising
	 *
	 * @return the number of advertised services a discovered BLE device is advertising
	 */
	uint8_t advertisedServiceUuidCount();

	/**
	 * Query an advertised service UUID discovered BLE device is advertising
	 *
	 * @param index (optional) the index of the service UUID. Defaults to 0
	 * @return advertised service UUID (as a string)
	 */
	String advertisedServiceUuid(uint8_t index = 0);

	/**
	 * Connect to a BLE device
	 *
	 * @param timeout in milliseconds
	 * @return true if the connection was successful
	 * @return false otherwise
	 */
	bool connect(uint32_t timeout = 5000);

	/**
	 * Find an advertisement of type type in the scanned advertisement data
	 *
	 * @param[in] type the type of ad to look for
	 * @param[out] foundData pointer to found ad + length of the ad, if present
	 * @return true if the advertisement contains an ad of type type
	 * @return false otherwise
	 */
	bool findAdvertisementDataType(GapAdvType type, ble_ad_t* foundData);

	/**
	 * Keep the connection with this central device alive by calling this every once in a while
	 */
	void connectionKeepAlive();
};
