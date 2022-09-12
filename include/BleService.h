#pragma once

#include <BleUtils.h>
#include <BleCharacteristic.h>
#include <String.h>
#include <microapp.h>

class BleService {

private:
	/*
	 * Allow full access for Ble class
	 */
	friend class Ble;

	bool _customUuid = false;
	bool _initialized = false;

	UUID128Bit _uuid128;

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
	void addCharacteristic(BleCharacteristic characteristic);

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
	 * @param index (optional) index of characteristic to check if the device provides more than one. Defaults to 0
	 * @return true if the service provides the characteristic
	 * @return false otherwise
	 */
	bool hasCharacteristic(const char* uuid, int index = 0);

	/**
	 * Get a BleCharacteristic representing a BLE characteristic the service provides
	 *
	 * @param uuid UUID of the characteristic as a string
	 * @return BleCharacteristic belonging to the provided uuid
	 */
	BleCharacteristic characteristic(const char* uuid);
};