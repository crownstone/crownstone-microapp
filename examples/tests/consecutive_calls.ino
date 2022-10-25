#include <Arduino.h>

const uint8_t CALL_LIMIT = 8;

void setup() {
	Serial.println("Consecutive call test");

	// We make one call more than is allowed (plus one in initial print)
	for (int i = 0; i <= CALL_LIMIT; i++) {
		Serial.println(i);
	}
}

void loop() {
}
