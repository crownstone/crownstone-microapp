#include <Arduino.h>

/**
 * Turns a LED on for one second, then off for one second, repeatedly.
 *
 * This example should be run on a 'crownstone' equipped with LEDs,
 * e.g. one of the nRF52 development kits
 * Most 'real' crownstones are not equipped with LEDs,
 * so this example will not do anything for these.
 */

// Which LED to use.
// LEDs 3 and 4 are reserved for relay and dimmer on PCA10040 boards.
const uint8_t ledPin = LED1_PIN;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Blinky example");

	// Set LED to OUTPUT, so we can write.
	pinMode(ledPin, OUTPUT);
}

void loop() {

	// Turn the LED on.
	digitalWrite(ledPin, true);

	// Keep the LED on for a second.
	delay(1000);

	// Turn the LED off.
	digitalWrite(ledPin, false);

	// Keep the LED off for a second.
	delay(1000);
}

