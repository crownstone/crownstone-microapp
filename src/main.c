#include <Arduino.h>
#include <ipc/cs_IpcRamData.h>
#include <microapp.h>


#ifdef __cplusplus
extern "C" {
#endif

#if __SIZEOF_POINTER__ != 4
#warning "Incorrect uintptr_t type"
#endif

void delay(uint16_t delay_ms) {
	const uint8_t bluenet_ticks = 100; // 100 ms
	uint8_t ticks = delay_ms / bluenet_ticks;
	for (int i = 0; i < ticks; i++) {
		microapp_delay_cmd_t *delay_cmd = (microapp_delay_cmd_t*)&global_msg;
		delay_cmd->cmd = CS_MICROAPP_COMMAND_DELAY;
		delay_cmd->period = ticks;
		global_msg.length = sizeof(microapp_delay_cmd_t);
		sendMessage(&global_msg);
	}
}

void signalSetupEnd() {
	global_msg.payload[0] = CS_MICROAPP_COMMAND_SETUP_END;
	global_msg.length = 1;
	sendMessage(&global_msg);
}

void signalLoopEnd() {
	global_msg.payload[0] = CS_MICROAPP_COMMAND_LOOP_END;
	global_msg.length = 1;
	sendMessage(&global_msg);
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
	setup();
	signalSetupEnd();
	while(1) {
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

/*
 * We will enter from the Reset_Handler. This is the very first instruction as defined in the assembly file startup.S.
 */
__attribute__((weak)) void _start(void){
	main();
}

#ifdef __cplusplus
}
#endif

