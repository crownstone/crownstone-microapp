#include <Arduino.h>
#include <ServiceData.h>

/*
 * This app will crash bluenet!
 * It is meant to steadily increase the stack used by the microapp to explore
 * when bluenet will crash
 */

void foo(uint32_t i) {
	// bla adds 64 bytes on the stack
	// i is an additional 4 bytes
	// and i + 1 is probably another?
	// In any case, every iteration will add 72 bytes in total
	// This can be seen through the difference in the bla pointer printed every iteration
	uint32_t bla[16];
	bla[0] = i;
	Serial.println((unsigned int)&bla);
	Serial.println((unsigned int)i);
	Serial.println("------");
	foo(i+1);
}

void setup() {
	Serial.println("Stack overflow microapp");

	uint32_t i = 0;
	foo(i);
}

void loop() {}
