#include <Arduino.h>
#include <ipc/cs_IpcRamData.h>
#include <microapp.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __SIZEOF_POINTER__ != 4
#warning "Incorrect uintptr_t type"
#endif

/*
 * Yield to bluenet and indicate end of setup
 */
void signalSetupEnd() {
	uint8_t* payload            = getOutgoingMessagePayload();
	microapp_sdk_yield_t* yield = reinterpret_cast<microapp_sdk_yield_t*>(payload);
	yield->header.ack           = CS_MICROAPP_SDK_ACK_NO_REQUEST;
	yield->header.messageType   = CS_MICROAPP_SDK_TYPE_YIELD;
	yield->type                 = CS_MICROAPP_SDK_YIELD_SETUP;
	yield->emptyInterruptSlots  = emptySlotsInStack();
	sendMessage();
}

/*
 * Yield to bluenet and indicate end of loop
 */
void signalLoopEnd() {
	uint8_t* payload            = getOutgoingMessagePayload();
	microapp_sdk_yield_t* yield = reinterpret_cast<microapp_sdk_yield_t*>(payload);
	yield->header.ack           = CS_MICROAPP_SDK_ACK_NO_REQUEST;
	yield->header.messageType   = CS_MICROAPP_SDK_TYPE_YIELD;
	yield->type                 = CS_MICROAPP_SDK_YIELD_LOOP;
	yield->emptyInterruptSlots  = emptySlotsInStack();
	sendMessage();
}

/*
 * This function is called from main and not directly anymore.
 *
 * It is absolutely essential that sendMessage is called regularly. Hence this is added to the end of setup and the end
 * of each loop call though signalSetupEnd() and signalLoopEnd();
 * If this is not done the coroutine finishes and bluenet will crash.
 */
int __attribute__((optimize("O0"))) dummy_main() {
	setup();
	signalSetupEnd();
	while (1) {
		loop();
		signalLoopEnd();
	}
	// will not be reached
	return -1;
}

/*
 * We will just pass through to dummy_main. Let's keep this function in case we want to jump to it in a later stage.
 */
int main() {
	dummy_main();
	// will not be reached
	return -1;
}

// provided by the linker
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

void __libc_init_array (void)
{
  size_t count;
  size_t i;

  count = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < count; i++)
    __preinit_array_start[i] ();

  // normally here a call to custom _init() function

  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i] ();
}


/*
 * We will enter from the Reset_Handler. This is the very first instruction as defined in the assembly file startup.S.
 */
__attribute__((weak)) void _start(void) {
	__libc_init_array();
	main();
}

#ifdef __cplusplus
}
#endif
