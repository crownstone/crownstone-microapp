#include <Arduino.h>

/**
 * A very basic microapp example for toggling LEDs.
 *
 * This example should be run on a 'crownstone' equipped with LEDs,
 * e.g. one of the nRF52 development kits
 * Most 'real' crownstones are not equipped with LEDs,
 * so this example will not do anything for these.
 */

// We use the first two leds here.
// Leds 3 and 4 are reserved for relay and dimmer on PCA10040 boards.
const uint8_t nrLEDs = 2;
boolean ledState[nrLEDs];
const uint8_t ledPins[nrLEDs] = {LED1_PIN, LED2_PIN};
uint8_t counter = 0;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Blinky example");

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

