#pragma once

#include <BleCharacteristic.h>
#include <BleUtils.h>
#include <BleUuid.h>
#include <String.h>
#include <microapp.h>

#ifndef MAX_CHARACTERISTICS_PER_SERVICE
#define MAX_CHARACTERISTICS_PER_SERVICE 6
#endif

class BleService {

private:
	/*
	 * Allow full access for Ble classes
	 */
	friend class Ble;
	friend class BleDevice;

	// Constructor for remote (discovered) service
	BleService(microapp_sdk_ble_uuid_t* uuid);

	struct {
		//! whether characteristic is empty or not
		bool initialized = false;
		//! whether characteristic is local or remote
		bool remote = false;
		//! (only for local service) whether service has been added to bluenet
		bool added = false;
	} _flags;

	Uuid _uuid;
	uint16_t _handle = 0;

	BleCharacteristic* _characteristics[MAX_CHARACTERISTICS_PER_SERVICE];
	uint8_t _characteristicCount = 0;

	/**
	 * Add local service and its characteristics via calls to bluenet
	 *
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if service not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if service is not local but remote
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t addLocalService();

	/**
	 * Get a characteristic based on its handle (value or cccd)
	 *
	 * @param[in] handle the value or cccd handle of the characteristic
	 * @param[out] characteristic if found, pointer to characteristic pointer will be placed here
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if service not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND if characteristic is not found
	 * @return microapp_sdk_result_t specifying other error
	 */
	microapp_sdk_result_t getCharacteristic(uint16_t handle, BleCharacteristic** characteristic);

	/**
	 * Internally add a remote discovered characteristic
	 *
	 * @param characteristic pointer to a discovered characteristic
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success
	 * @return CS_MICROAPP_SDK_ACK_ERR_EMPTY if service not initialized
	 * @return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED if service is not remote but local
	 * @return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE if there is no space for new characteristics
	 */
	microapp_sdk_result_t addDiscoveredCharacteristic(BleCharacteristic* characteristic);

public:
	// Empty constructor
	BleService(){};

	explicit operator bool() const;

	/**
	 * Create a new (local) BLE service
	 *
	 * @param[in] uuid 16-bit or 128-bit UUID in string format
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
	uint8_t characteristicCount();

	/**
	 * Query if the BLE service has a particular characteristic
	 *
	 * @param[in] uuid UUID of the characteristic to check as a string
	 * @return true if the service provides the characteristic
	 * @return false otherwise
	 */
	bool hasCharacteristic(const char* uuid);

	/**
	 * Get a BleCharacteristic representing a BLE characteristic the service provides
	 *
	 * @param[in] uuid UUID of the characteristic as a string
	 * @param[in] index index of the characteristic to look for
	 * @return BleCharacteristic belonging to the provided uuid
	 */
	BleCharacteristic& characteristic(const char* uuid);
	BleCharacteristic& characteristic(uint8_t index);
};