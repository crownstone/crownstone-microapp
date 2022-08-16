#include <Arduino.h>

/**
 * A very basic microapp example for toggling LEDs.
 *
 * This example should be run on a 'crownstone' equipped with LEDs,
 * e.g. one of the nRF52 development kits
 * Most 'real' crownstones are not equipped with LEDs,
 * so this example will not do anything for these.
 */

const uint8_t nrLEDs = 3;
boolean ledState[nrLEDs];
const uint8_t ledPins[nrLEDs] = {LED1_PIN, LED2_PIN, LED3_PIN};
uint8_t counter = 0;

void setup() {
	// Set LEDs to OUTPUT, so we can write.
	for (int i = 0; i < nrLEDs; i++) {
		ledState[i] = false;
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

