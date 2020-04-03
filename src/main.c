#include <ipc/cs_IpcRamData.h>
#include <Arduino.h>

void __attribute__((optimize("O0"))) dummy_main() {
	uint8_t len = 8;
	unsigned char buf[8] = "arduino";
	buf[7] = 0;
//	std::string name("arduino");
//	len = name.length();
	setRamData(IPC_INDEX_ARDUINO_APP, buf, len);
}



int main(int argc, char *argv[]) {
	dummy_main();

	// assume callback, mmm, no, we can't do that like this
	// our stack is trashed by just setting stack pointer, we have to jump back


	// reboot(?)
	return -1; 
}
