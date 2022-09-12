#include <BleService.h>

BleService::BleService(const char* uuid) {
	size_t len = strlen(uuid);
	if (len != UUID_16BIT_STRING_LENGTH && len != UUID_128BIT_STRING_LENGTH) {
		return;
	}
	if (len == UUID_128BIT_STRING_LENGTH) {
		_customUuid = true;
	}
	_uuid128 = UUID128Bit(uuid);
	_initialized = true;
}