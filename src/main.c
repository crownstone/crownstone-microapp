#include <Arduino.h>
#include <ipc/cs_IpcRamData.h>
#include <string.h>

#include <setjmp.h>

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

static int _delay_ms = 0;

typedef struct {
  jmp_buf callee_context;
  jmp_buf caller_context;
} coroutine;

coroutine cor;

int cont(coroutine *c);


int entry_func(char opcode, char* payload) {
	int result = 0;
	switch(opcode) {
	case 0: {
		setup();
		break;
	}
	case 1: {
		result = loop();
		break;
	}
	case 2: {
		result = cont(&cor);
		break;
	}
	default: {
		// unhandled opcode
		result = -1;
		
	}
	}
	return result;
}

//
// The implementation of delay is tough. We cannot just have some kind of entry function and either start a loop or
// jump directly at the end of some delay function. This would namely not preserve the stack. We can neither just 
// call into the bluenet code and then return to this code because that would put all kind of stuff on the stack in
// the bluenet code after running the microapp code. We should normally return from the call into the microapp code
// and not enter bluenet code via a different path. 
//
// What would be ideal is to put certain tasks on the app_scheduler for later execution. Hence, what we need is to
// write towards the bluenet code a task that has to be executed later and then return normally. Mmm, but how do we
// resume then?
//
// No, we should implement two stacks and jump back and forth... How to do this? First, we can have the stack grow
// downwards from RAM_END as defined for this microapp. Then we use setjmp/longjmp to implement coroutines.
//
void delay(uint16_t delay_ms) {
	_delay_ms = delay_ms;

//	cont(&cor);

}

enum { WORKING=1, DONE };

int cont(coroutine *c) {
	if(!setjmp(c->callee_context)) {
		longjmp(c->caller_context, WORKING);
	}
	return 0;
}

/*
 * We assume the following protocol
 *
 * [version] [setup] [loop]
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

	// address of loop() function
	address = (uintptr_t)&loop;
	for (uint16_t i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);

	/*
	// address of entry_func() function
	address = (uintptr_t)&entry_func;
	for (uint16_t i = 0; i < sizeof(uintptr_t); ++i) {
		buf[i+len] = (uint8_t)(0xFF & (address >> (i*8)));
	}
	len += sizeof(uintptr_t);
	*/
	// set buffer in RAM
	setRamData(IPC_INDEX_MICROAPP, buf, len);

	return (int)&setup;
}

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[]) {
	dummy_main();

	// assume callback, mmm, no, we can't do that like this
	// our stack is trashed by just setting stack pointer, we have to jump back


	// reboot(?)
	return -1; 
}

