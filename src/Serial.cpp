#include <Serial.h>
#include <ipc/cs_IpcRamData.h>

#include <stdint.h>

// for now maximum payload of 32 bytes
const uint8_t MAX_PAYLOAD = 32;

const uint8_t SIZE_OPCODE = 1;

// this will then also limit maximum string size
const uint8_t MAX_STRING_LENGTH = MAX_PAYLOAD - SIZE_OPCODE;

// the payload is reused for every function call
uint8_t payload[MAX_PAYLOAD];

static int g_err_code = 0;

// returns size 0 for strings that are too long
uint8_t strlen(const char *str) {
	for (uint8_t i = 0; i < MAX_STRING_LENGTH; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return 0;
}

void Serial_::write(char value) {
	const char buf[1] = { value };
	write(buf, 1);
}

int Serial_::write(const char *str) {
	return write(str, strlen(str));
}

int Serial_::write(String str, int length) {
	return write(str.c_str(), length);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int Serial_::write(const char *str, int length) {
	if (length > MAX_STRING_LENGTH) {
		length = MAX_STRING_LENGTH;
	}
	
	uint8_t buf[BLUENET_IPC_RAM_DATA_ITEM_SIZE];
	for (int i = 0; i < BLUENET_IPC_RAM_DATA_ITEM_SIZE; ++i) {
		buf[i] = 0;
	}
	uint8_t rd_size = 0;
	uint8_t ret_code = getRamData(IPC_INDEX_CROWNSTONE_APP, buf, BLUENET_IPC_RAM_DATA_ITEM_SIZE, &rd_size);

	bluenet_ipc_ram_data_item_t *ramStr = getRamStruct(IPC_INDEX_MICROAPP);
	if (!ramStr) {
		return -1;
	}

	uintptr_t _callback = 0;

	if (ret_code == IPC_RET_SUCCESS) {
		uint8_t protocol = buf[0];
		if (protocol == 0) {
			uint8_t offset = 1;
			for (int i = 0; i < 4; ++i) {
				_callback = _callback | ( (uintptr_t)(buf[i+offset]) << (i*8));
			}
		}
	}

	for (int i = 0; i < length; ++i) {
		payload[i + 1] = str[i];
	}
	payload[0] = 1;
	if (_callback) {
		int (*callback_func)(char*,uint16_t) = (int (*)(char*,uint16_t)) _callback;
		g_err_code = callback_func((char*)payload, length + 1);
	}
	return length;
}
