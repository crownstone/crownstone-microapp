#pragma once

#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

class BleCharacteristicProperties {
public:
	// Bit 0 reserved for BLEBroadcast
	static const uint8_t BLERead                 = 1 << 1;
	static const uint8_t BLEWriteWithoutResponse = 1 << 2;
	static const uint8_t BLEWrite                = 1 << 3;
	static const uint8_t BLENotify               = 1 << 4;
	static const uint8_t BLEIndicate             = 1 << 5;
};

// Forward declarations
class BleCharacteristic;
class BleDevice;

typedef void (*CharacteristicEventHandler)(BleDevice, BleCharacteristic);

class BleCharacteristic {

private:
	/*
	 * Allow full access for Ble classes
	 */
	friend class Ble;
	friend class BleDevice;
	friend class BleService;

	// default constructor
	BleCharacteristic(){};

	// Constructor for remote characteristics
	BleCharacteristic(microapp_sdk_ble_uuid_t* uuid, uint8_t properties);

	static constexpr uint16_t MAX_CHARACTERISTIC_VALUE_SIZE = 256;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			bool initialized  : 1; // whether characteristic is empty or not
			bool remote       : 1; // whether characteristic is local or remote
			bool added        : 1; // (only for local characteristics) whether characteristic has been added to bluenet
			bool subscribed   : 1; // (only for local characteristics) whether characteristic is subscribed to
			bool written      : 1; // (only for local characteristics) whether characteristic is written to
			bool valueUpdated : 1; // (only for remote characteristics) whether EVENT_NOTIFICATION has happened
			bool valueRead    : 1; // (only for remote characteristics) whether EVENT_READ has happened
			bool valueWritten : 1; // (only for remote characteristics) whether EVENT_WRITE has happened
			bool notificationDone : 1; // (only for local characteristics) whether notification is done
		} flags;
		uint16_t asInt = 0;  // initialize to zero
	} _flags;

	uint8_t _properties   = 0;
	uint8_t* _value       = nullptr;
	uint16_t _valueSize   = 0;
	uint16_t _valueLength = 0;
	uint16_t _handle      = 0;

	Uuid _uuid;

	/**
	 * Add characteristic via call to bluenet (only for local characteristics)
	 *
	 * @param[in] serviceHandle handle of the service
	 * @return microapp_sdk_result_t
	 */
	microapp_sdk_result_t add(uint16_t serviceHandle);

	/**
	 * Register a custom characteristic uuid via a call to bluenet
	 *
	 * @return microapp_sdk_result_t
	 */
	microapp_sdk_result_t registerCustomUuid();

	microapp_sdk_result_t writeValueLocal(uint8_t* buffer, uint16_t length);

	microapp_sdk_result_t writeValueRemote(uint8_t* buffer, uint16_t length);

	microapp_sdk_result_t notify();

	microapp_sdk_result_t readValueRemote(uint8_t* buffer, uint16_t length);

public:

	explicit operator bool() const { return _flags.flags.initialized; }

	/**
	 * Create a new BLE characteristic
	 *
	 * @param uuid 16-bit or 128-bit UUID in string format
	 * @param properties mask of the properties in BleCharacteristicProperties
	 * @param value byte array where value is stored
	 * @param valueSize (maximum) size of characteristic value
	 */
	BleCharacteristic(const char* uuid, uint8_t properties, uint8_t* value, uint16_t valueSize);

	/**
	 * Query the UUID of the specified BleCharacteristic
	 *
	 * @return UUID of the BLE characteristic as a string
	 */
	String uuid();

	/**
	 * Query the property mask of the specified BLECharacteristic
	 *
	 * @return masked byte with bits representing BleCharacteristicProperties
	 */
	uint8_t properties();

	/**
	 * Query the maximum value size of the specified BLECharacteristic
	 *
	 * @return the maximum value size of the characteristic in bytes
	 */
	uint16_t valueSize();

	/**
	 * Query the current value of the specified BLECharacteristic
	 *
	 * @return The current value of the characteristic, as a byte buffer
	 */
	uint8_t* value();

	/**
	 * Query the current value size of the specified BLECharacteristic
	 *
	 * @return the current value size of the characteristic (in bytes)
	 */
	uint16_t valueLength();

	/**
	 * Read the current value of the characteristic.
	 * If the characteristic is on a remote device, a read request will be sent
	 *
	 * @param[in] buffer byte array to read value into
	 * @param[in] length size of buffer argument in bytes
	 * @return number of bytes read
	 */
	uint16_t readValue(uint8_t* buffer, uint16_t length);

	/**
	 * Write the value of the characteristic
	 *
	 * @param buffer byte array to write value with
	 * @param length number of bytes of the buffer argument to write
	 * @return true on success
	 * @return false on failure
	 */
	bool writeValue(uint8_t* buffer, uint16_t length);

	/**
	 * Set the event handler (callback) function that will be called when the specified event occurs
	 *
	 * @param eventType event type (BLESubscribed, BLEUnsubscribed, BLERead, BLEWritten)
	 * @param eventHandler function to call when the event occurs
	 */
	void setEventHandler(BleEventType eventType, CharacteristicEventHandler eventHandler);

	/**
	 * Query if the characteristic value has been written by another BLE device
	 *
	 * @return true if the characteristic value has been written
	 * @return false otherwise
	 */
	bool written();

	/**
	 * Query if the characteristic has been subscribed to by another BLE device
	 *
	 * @return true if the characteristic value has been subscribed to
	 * @return false otherwise
	 */
	bool subscribed();

	/**
	 * Query if a BLE characteristic is readable
	 *
	 * @return true if characteristic is readable
	 * @return false otherwise
	 */
	bool canRead();

	/**
	 * Query if a BLE characteristic is writable
	 *
	 * @return true if characteristic is writable
	 * @return false otherwise
	 */
	bool canWrite();

	/**
	 * Query if a BLE characteristic is subscribable
	 *
	 * @return true if characteristic is subscribable
	 * @return false otherwise
	 */
	bool canSubscribe();

	/**
	 * Subscribe to a BLE characteristic notifications or indications
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool subscribe();

	/**
	 * Query if a BLE characteristic is unsubscribable
	 *
	 * @return true if characteristic is unsubscribable
	 * @return false otherwise
	 */
	bool canUnsubscribe();

	/**
	 * Unsubscribe to a BLE characteristic notifications or indications
	 *
	 * @return true on success
	 * @return false on failure
	 */
	bool unsubscribe();

	/**
	 * Has the characteristics value been updated via a notification or indication
	 *
	 * @return true if characteristic has been updated via a notification or indication
	 * @return false otherwise
	 */
	bool valueUpdated();
};
