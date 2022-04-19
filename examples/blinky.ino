//
// Blinking LEDs
//

#include <Arduino.h>

const uint8_t nrLEDs = 3;
boolean ledState[nrLEDs] = {LOW};
const uint8_t ledPins[nrLEDs] = {LED1_PIN, LED2_PIN, LED3_PIN};
uint8_t counter = 0;

void setup() {
	// Set LEDs to OUTPUT, so we can write.
	for (int i = 0; i < nrLEDs; i++) {
		pinMode(ledPins[i], OUTPUT);
	}
}

void loop() {

	// get the index of the led to be toggled this cycle
	uint8_t ledIndex = counter % nrLEDs;

	// toggle the led state of the given led
	ledState[ledIndex] = !ledState[ledIndex];

	// write the new led state
	digitalWrite(ledPins[ledIndex], ledState[ledIndex]);

	// increment the loop counter
	counter++;
}
