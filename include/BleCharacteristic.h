#pragma once

#include <BleDevice.h>
#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

class BleCharacteristicProperties {
public:
	static const uint8_t BLEBroadcast            = 1 << 0;
	static const uint8_t BLERead                 = 1 << 1;
	static const uint8_t BLEWriteWithoutResponse = 1 << 2;
	static const uint8_t BLEWrite                = 1 << 3;
	static const uint8_t BLENotify               = 1 << 4;
	static const uint8_t BLEIndicate             = 1 << 5;
};

class BleCharacteristic {

private:
	/*
	 * Allow full access for Ble class and BleService class
	 */
	friend class Ble;
	friend class BleService;

	// default constructor
	BleCharacteristic(){};

	// Same as public constructor except allows for setting remote flag
	BleCharacteristic(const char* uuid, uint8_t properties, bool remote);

	bool _initialized   = false;
	bool _remote        = false;
	bool _subscribed    = false;
	bool _written       = false;
	uint8_t _properties = 0;

	static const size_t MAX_CHARACTERISTIC_VALUE_SIZE = 256;
	uint8_t* _value                                   = nullptr;
	size_t _valueLength                               = 0;

	bool _customUuid = false;
	UUID128Bit _uuid;

public:
	/**
	 * Create a new BLE service
	 *
	 * @param uuid 16-bit or 128-bit UUID in string format
	 */
	BleCharacteristic(const char* uuid, uint8_t properties);

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
	int valueSize();

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
	int valueLength();

	/**
	 * Write the value of the characteristic
	 *
	 * @param buffer byte array to write value with
	 * @param length number of bytes of the buffer argument to write
	 * @return true on success
	 * @return false on failure
	 */
	bool writeValue(uint8_t* buffer, size_t length);

	/**
	 * Set the event handler (callback) function that will be called when the specified event occurs
	 *
	 * @param eventType event type (BLESubscribed, BLEUnsubscribed, BLERead, BLEWritten)
	 * @param eventHandler function to call when the event occurs
	 */
	void setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice, BleCharacteristic));

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
};
