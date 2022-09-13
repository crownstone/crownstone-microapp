#pragma once

#include <BleCharacteristic.h>
#include <BleUtils.h>
#include <String.h>
#include <microapp.h>

class BleService {

private:
	/*
	 * Allow full access for Ble class
	 */
	friend class Ble;

	// Default constructor
	BleService(){};

	// Same as public constructor except allows for setting remote flag
	BleService(const char* uuid, bool remote);

	bool _customUuid  = false;
	bool _initialized = false;
	bool _remote      = false;

	UUID128Bit _uuid128;

	static const uint8_t MAX_CHARACTERISTICS = 6;
	BleCharacteristic* _characteristics[MAX_CHARACTERISTICS];
	uint8_t _characteristicCount = 0;

public:
	/**
	 * Create a new BLE service
	 *
	 * @param uuid 16-bit or 128-bit UUID in string format
	 */
	BleService(const char* uuid);

	/**
	 * Query the UUID of the specified BleService
	 *
	 * @return UUID of the BLE service as a string
	 */
	String uuid();

	/**
	 * Add a BleCharacteristic to the BLE service
	 *
	 * @param characteristic BleCharacteristic to add
	 */
	void addCharacteristic(BleCharacteristic& characteristic);

	/**
	 * Query the number of characteristics discovered for the BLE service
	 *
	 * @return The number of characteristics
	 */
	int characteristicCount();

	/**
	 * Query if the BLE service has a particular characteristic
	 *
	 * @param uuid UUID of the characteristic to check as a string
	 * @return true if the service provides the characteristic
	 * @return false otherwise
	 */
	bool hasCharacteristic(const char* uuid);

	/**
	 * Get a BleCharacteristic representing a BLE characteristic the service provides
	 *
	 * @param uuid UUID of the characteristic as a string
	 * @return BleCharacteristic belonging to the provided uuid
	 */
	BleCharacteristic characteristic(const char* uuid);
};