#include <Arduino.h>
#include <ipc/cs_IpcRamData.h>
#include <microapp.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __SIZEOF_POINTER__ != 4
#warning "Incorrect uintptr_t type"
#endif

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

void* _coroutine_args;

void goyield(uint16_t prefix) {

	microapp_delay_cmd_t *delay_cmd = (microapp_delay_cmd_t*)&global_msg;

	delay_cmd->cmd = CS_MICROAPP_COMMAND_DELAY;
	delay_cmd->period = prefix;
	delay_cmd->coargs = (uintptr_t)_coroutine_args;

	global_msg.length = sizeof(microapp_delay_cmd_t);

	sendMessage(&global_msg);
}

void delay(uint16_t delay_ms) {
	goyield(delay_ms);
}

/*
 * We wrap loop() to store the coroutine arguments and be able to pass it back to the caller in e.g. the delay
 * function.
 */
void coloop(void *p) {
	_coroutine_args = p;
	loop();
}

/*
 * We assume the following protocol
 *
 * [version] [setup] [coloop]
 *
 * Version is only one byte. The addresses of setup and loop are of size uniptr_t.
 * Note that you are not allowed to change the signature of those functions! 
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
	for (uint16_t i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);

	// address of coroutine which calls the loop() function
	address = (uintptr_t)&coloop;
	for (uint16_t i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);

	// set buffer in RAM
	setRamData(IPC_INDEX_MICROAPP, buf, len);

	return (int)&setup;
}

#ifdef __cplusplus
}
#endif

/**
 * We actually do not "use" the main function. The bluenet code will immmediately jump to dummy_main. The function 
 * also does not need to return. This function exists just to make the compiler happy.
 */
int main(int argc, char *argv[]) {
	dummy_main();
	return -1;
}

