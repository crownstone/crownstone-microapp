#include <ipc/cs_IpcRamData.h>
#include <Arduino.h>

#if __SIZEOF_POINTER__ != 4
#warning "Incorrect uintptr_t type"
#endif

int __attribute__((optimize("O0"))) dummy_main() {
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


int main(int argc, char *argv[]) {
	dummy_main();

	// assume callback, mmm, no, we can't do that like this
	// our stack is trashed by just setting stack pointer, we have to jump back


	// reboot(?)
	return -1; 
}
