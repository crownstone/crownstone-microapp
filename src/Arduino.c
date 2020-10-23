#include <Arduino.h>
#include <microapp.h>
#include <ipc/cs_IpcRamData.h>

void pinMode(uint8_t pin, uint8_t mode){
}

void digitalWrite(uint8_t pin, uint8_t val) {

	global_msg.payload[0] = 3;
	global_msg.payload[1] = pin;
	global_msg.payload[2] = val;
	global_msg.length = 3;

	sendMessage(global_msg);
}

int digitalRead(uint8_t pin){
	return 0;
}

int analogRead(uint8_t pin){
	return 0;
}

void analogReference(uint8_t mode){
}

void analogWrite(uint8_t pin, int val){
}

void init() {
}

void initVariant() {
}
