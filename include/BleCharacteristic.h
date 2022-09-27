#pragma once

#include <BleUuid.h>
#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

// This should be replaced when multiple connections are possible in bluenet
// For now, it is okay to have it be 0 always
#define BLE_CONNECTION_HANDLE_PLACEHOLDER 0

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
bool registeredBleInterrupt(MicroappSdkBleType bleType);
microapp_sdk_result_t registerBleInterrupt(MicroappSdkBleType bleType);
microapp_sdk_result_t registerBleEventHandler(BleEventType eventType, BleEventHandler eventHandler);
microapp_sdk_result_t getBleEventHandlerRegistration(BleEventType eventType, BleEventHandlerRegistration& registration);
microapp_sdk_result_t removeBleEventHandlerRegistration(BleEventType eventType);


/**
 * Class for storing a characteristic. A characteristic is a container for data.
 * The class can be used for both remote characteristics (i.e. data exists on a remote peripheral device)
 * and for local characteristic (i.e. the crownstone is the peripheral)
 */
class BleCharacteristic {

private:
	/*
	 * Allow full access for Ble classes
	 */
	friend class Ble;
	friend class BleDevice;
	friend class BleService;

	// Default constructor
	BleCharacteristic(){};

	// Constructor for remote characteristics
	BleCharacteristic(microapp_sdk_ble_uuid_t* uuid, uint8_t properties);

	static constexpr uint16_t MAX_CHARACTERISTIC_VALUE_SIZE = 256;

	union __attribute__((packed)) flags_t {
		struct __attribute__((packed)) {
			//! whether characteristic is empty or not
			bool initialized : 1;
			//! whether characteristic is local or remote
			bool remote : 1;
			//! (only for local characteristics) whether characteristic has been added to bluenet
			bool added : 1;
			//! (only for local characteristics) whether characteristic is subscribed to
			bool subscribed : 1;
			//! (only for local characteristics) whether characteristic is written to
			bool writtenAsLocal : 1;
			//! (only for local characteristics) whether notification is done
			bool localNotificationDone : 1;
			//! (only for remote characteristics) whether EVENT_NOTIFICATION has happened
			bool remoteValueUpdated : 1;
			//! (only for remote characteristics) whether EVENT_READ has happened
			bool remoteValueRead : 1;
			//! (only for remote characteristics) whether EVENT_WRITE has happened
			bool writtenToRemote : 1;
		} flags;
		// initialize to zero
		uint16_t asInt = 0;
	} _flags;

	uint8_t _properties   = 0;
	uint8_t* _value       = nullptr;
	uint16_t _valueSize   = 0;
	uint16_t _valueLength = 0;
	uint16_t _handle      = 0;

	Uuid _uuid;

	/**
	 * Add local characteristic via call to bluenet (only for local characteristics)
	 *
	 * @param[in] serviceHandle handle of the service
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BleCharacteristic not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if BleCharacteristic is not local but remote
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t addLocalCharacteristic(uint16_t serviceHandle);

	/**
	 * Register the local characteristics custom uuid via a call to bluenet
	 *
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BleCharacteristic not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if BleCharacteristic is not local but remote or uuid is not custom
	 * @return CS_MICROAPP_SDK_ACK_ERROR if bluenet returned different uuid than the original
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t registerCustomUuid();

	/**
	 * Write value to a local characteristic and lets bluenet know
	 * Sends a VALUE_SET request to bluenet and if subscribed, calls notify()
	 *
	 * @param buffer buffer to write from
	 * @param length length of the buffer
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BleCharacteristic not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if BleCharacteristic is not local but remote
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t writeValueLocal(uint8_t* buffer, uint16_t length);

	/**
	 * Write value to a remote characteristic
	 * Sends a WRITE request to bluenet and waits for WRITE event back
	 *
	 * @param buffer buffer to write in
	 * @param length length of the buffer
	 * @param timeout in milliseconds
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BleCharacteristic not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if BleCharacteristic is not remote but local
	 * @return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT if no (valid) WRITE event is received within timeout
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t writeValueRemote(uint8_t* buffer, uint16_t length, uint32_t timeout = 5000);

	/**
	 * Reads value from a remote characteristic
	 * Sends a READ request to bluenet and waits for READ event back
	 *
	 * @param buffer buffer to read value to
	 * @param length (max) length of buffer to write the read value to
	 * @param timeout in milliseconds
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if BleCharacteristic not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if BleCharacteristic is not remote but local
	 * @return CS_MICROAPP_SDK_ACK_ERR_TIMEOUT if no (valid) READ event is received within timeout
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t readValueRemote(uint8_t* buffer, uint16_t length, uint32_t timeout = 5000);


	microapp_sdk_result_t onRemoteWritten();
	microapp_sdk_result_t onRemoteRead(microapp_sdk_ble_central_event_read_t* eventRead);
	microapp_sdk_result_t onRemoteNotification(microapp_sdk_ble_central_event_notification_t* eventNotification);

	microapp_sdk_result_t onLocalWritten();
	microapp_sdk_result_t onLocalSubscribed();
	microapp_sdk_result_t onLocalUnsubscribed();
	microapp_sdk_result_t onLocalNotificationDone();

public:
	explicit operator bool() const;

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
	 * @param timeout in milliseconds
	 * @return true on success
	 * @return false on failure
	 */
	bool subscribe(uint32_t timeout = 5000);

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
	 * @param timeout in milliseconds
	 * @return true on success
	 * @return false on failure
	 */
	bool unsubscribe(uint32_t timeout = 5000);

	/**
	 * Has the characteristics value been updated via a notification or indication
	 *
	 * @return true if characteristic has been updated via a notification or indication
	 * @return false otherwise
	 */
	bool valueUpdated();
};
