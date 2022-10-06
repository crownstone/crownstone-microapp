#pragma once

#include <BleUtils.h>
#include <microapp.h>

// type for 16-bit uuid
typedef uint16_t uuid16_t;
// amount of bytes in a 16-bit uuid
const microapp_size_t UUID_16BIT_BYTE_LENGTH = 2;
// format "ABCD"
const microapp_size_t UUID_16BIT_STRING_LENGTH = 4;
// amount of bytes in a 128-bit uuid
const microapp_size_t UUID_128BIT_BYTE_LENGTH = 16;
// format "12345678-ABCD-1234-5678-ABCDEF123456"
const microapp_size_t UUID_128BIT_STRING_LENGTH = 36;

class Uuid {
private:
	friend class Ble;
	friend class BleDevice;
	friend class BleService;
	friend class BleCharacteristic;

	static constexpr uint8_t BASE_UUID_128BIT[UUID_128BIT_BYTE_LENGTH] = {
			0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	static constexpr uint8_t BASE_UUID_OFFSET_16BIT = 12;

	uint8_t _uuid[UUID_128BIT_BYTE_LENGTH];
	uint8_t _length = 0;
	// after registration, bluenet passes an id for custom uuids
	uint8_t _type     = CS_MICROAPP_SDK_BLE_UUID_NONE;
	bool _initialized = false;

	/**
	 * Whether the uuid has been registered with bluenet.
	 * 16-bit uuids are registered by default so will always return true
	 *
	 * @return true if the uuid is registered with bluenet
	 * @return false if the uuid is custom (128-bit) and not registered with bluenet
	 */
	bool registered();

	/**
	 * If the uuid is not a standardized uuid or already registered, register uuid with bluenet
	 * Bluenet returns an assigned type which will be stored internally
	 *
	 * @return CS_MICROAPP_SDK_ACK_SUCCESS on success (also if already registered)
	 * @return CS_MICROAPP_SDK_ACK_ERROR if bluenet did not return same short uuid
	 * @return microapp_sdk_result_t with other code if bluenet failed to handle request
	 */
	microapp_sdk_result_t registerCustom();

	/**
	 * Set the type of the uuid which is relevant in bluenet context
	 * so a shortened uuid + type can be linked to the full uuid and vice versa
	 *
	 * @param type one of MicroappSdkBleUuidType
	 */
	void setType(uint8_t type);

	/**
	 * Get the type of the uuid which is relevant in bluenet context
	 * so a shortened uuid + type can be linked to the full uuid and vice versa
	 *
	 * @return type one of MicroappSdkBleUuidType
	 */
	uint8_t getType();

	/**
	 * Convert from 16-bit UUID string "ABCD" to 16-bit byte array
	 *
	 * @param[in] uuidString Null-terminated string of the format "ABCD", with either uppercase or lowercase letters.
	 * @param[out] emptyUuid A byte array where the resulting UUID will be placed
	 * @return true on success
	 * @return false if no valid string conversion could be made
	 */
	bool convertStringToUuid16Bit(const char* uuidString, uint8_t* emptyUuid);

	/**
	 * Convert from UUID string (either 16-bit or 128-bit) to 128-bit byte array
	 *
	 * @param[in] uuidString Null-terminated string of the format "12345678-ABCD-1234-5678-ABCDEF123456", with either
	 * uppercase or lowercase letters.
	 * @param[out] emptyUuid A byte array where the resulting UUID will be placed
	 * @return true on success
	 * @return false if no valid string conversion could be made
	 */
	bool convertStringToUuid128Bit(const char* uuidString, uint8_t* emptyUuid);

	/**
	 * Convert from 16-bit UUID to string representation in format "ABCD"
	 *
	 * @param[in] uuid             Pointer to byte array containing 16-bit UUID
	 * @param[out] emptyUuidString Char array where the stringified UUID will be placed
	 */
	void convertUuid16BitToString(const uint8_t* uuid, char* emptyUuidString);

	/**
	 * Convert from 128-bit UUID to string representation in format "12345678-ABCD-1234-5678-ABCDEF123456"
	 *
	 * @param[in] uuid             Pointer to byte array containing 128-bit UUID
	 * @param[out] emptyUuidString Char array where the stringified UUID will be placed
	 */
	void convertUuid128BitToString(const uint8_t* uuid, char* emptyUuidString);

public:
	Uuid(){};
	Uuid(const char* uuid);
	Uuid(const uint8_t* uuid, uint8_t length);
	Uuid(const uuid16_t uuid, uint8_t type);

	// comparison operators
	bool operator==(const Uuid& other);
	bool operator!=(const Uuid& other);

	// even though internally it's always 16 bytes, the length can be either 2 or 16
	uint8_t length();
	bool custom();
	bool valid();

	const char* string();
	// return full string, even for 16-bit uuids
	const char* fullString();

	const uint8_t* bytes();
	const uint8_t* fullBytes();

	// Returns a shortened 16-bit uint version of the uuid
	uuid16_t uuid16() const;
};