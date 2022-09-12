#pragma once

#include <ArduinoBLE.h>
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
	 * Allow full access for Ble class
	 */
	friend class Ble;

protected:
	union value_t {
		uint8_t byte;
	} _value;

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
	 * Set the event handler (callback) function that will be called when the specified event occurs
	 *
	 * @param eventType event type (BLESubscribed, BLEUnsubscribed, BLERead, BLEWritten)
	 * @param eventHandler function to call when the event occurs
	 */
	void setEventHandler(BleEventType eventType, void (*eventHandler)(BleDevice));

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

class BleByteCharacteristic : public BleCharacteristic {
public:
	BleByteCharacteristic(const char* uuid, uint8_t properties) : BleCharacteristic(uuid, properties){};
	BleByteCharacteristic(const char* uuid, uint8_t properties, uint8_t value) : BleCharacteristic(uuid, properties) {
		_value.byte = value;
	}

	/**
	 * Query the current value of the specified BLECharacteristic
	 *
	 * @return the current value of the characteristic, a byte
	 */
	uint8_t value() {
		return _value.byte;
	}

	/**
	 * Write the value of the characteristic
	 *
	 * @param value the value to write
	 * @return true on success
	 * @return false on failure
	 */
	bool writeValue(uint8_t value) {
		_value.byte = value;
		return true;
	}

};
