#include <Arduino.h>
#include <String.h>

const uint8_t CALL_LIMIT = 8;

void setup() {
	// We make one call more than is allowed
	for (int i = 0; i <= CALL_LIMIT; i++) {
		Serial.println(i);
	}
}

void loop() {
}
