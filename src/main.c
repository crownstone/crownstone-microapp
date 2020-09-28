#include <ipc/cs_IpcRamData.h>
#include <string.h>
#include <Arduino.h>

#if __SIZEOF_POINTER__ != 4
#warning "Incorrect uintptr_t type"
#endif

// for now maximum payload of 32 bytes
uint8_t payload[32];

// location of these variables is defined in linker script
extern unsigned __etext;
extern unsigned __data_start__;
extern unsigned __data_end__;


// initialize .data, can be moved to assembly later
void copy_data() {
    unsigned *src = &__etext;
    unsigned *dst = &__data_start__;
    while (dst < &__data_end__) {
        *dst++ = *src++;
    }
}

static int g_err_code = 0;

/*
 * We assume the following protocol
 *
 * [version] [setup] [loop]
 *
 * Version is only one byte. The addresses of setup and loop are of size uniptr_t.
 * Note that you are not allowed to change the signure of those functions! 
 */
int __attribute__((optimize("O0"))) dummy_main() {
	copy_data();

	uint8_t buf[BLUENET_IPC_RAM_DATA_ITEM_SIZE];

	// protocol version
	const char protocol_version = 0;
	buf[0] = protocol_version;
	uint8_t len = 1;

	// address of setup() function
	uintptr_t address = (uintptr_t)&setup;
	for (int i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);

	// address of loop() function
	address = (uintptr_t)&loop;
	for (int i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);

	// set buffer in RAM
	setRamData(IPC_INDEX_MICROAPP, buf, len);

	return (int)&setup;
}

void write(char *message, int length) {
	uint8_t buf[BLUENET_IPC_RAM_DATA_ITEM_SIZE];
	for (int i = 0; i < BLUENET_IPC_RAM_DATA_ITEM_SIZE; ++i) {
		buf[i] = 0;
	}
	uint8_t rd_size = 0;
	uint8_t ret_code = getRamData(IPC_INDEX_CROWNSTONE_APP, buf, BLUENET_IPC_RAM_DATA_ITEM_SIZE, &rd_size);

	bluenet_ipc_ram_data_item_t *ramStr = getRamStruct(IPC_INDEX_MICROAPP);
	if (!ramStr) {
		return;
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

	memcpy(&payload[1], message, length);
	payload[0] = 1;
	if (_callback) {
		int (*callback_func)(char*,uint16_t) = (int (*)(char*,uint16_t)) _callback;
		g_err_code = callback_func((char*)payload, length + 1);
	}
}

int main(int argc, char *argv[]) {
	dummy_main();

	// assume callback, mmm, no, we can't do that like this
	// our stack is trashed by just setting stack pointer, we have to jump back


	// reboot(?)
	return -1; 
}
