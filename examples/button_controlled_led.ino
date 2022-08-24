#include <Arduino.h>

/**
 * A basic microapp example showcasing interrupt functionality.
 *
 * This example should be run on a 'crownstone' equipped with buttons and LEDs,
 * e.g. one of the nRF52 development kits.
 * Most 'real' crownstones are not equipped with buttons and LEDs,
 * so this example will not do anything for these.
 */

boolean state = LOW;

// Callback for a button press
void blink() {
	Serial.println("blink");

	// Toggle LED
	state = !state;
	digitalWrite(LED1_PIN, state);
}

// In setup, configure button and LED GPIO pins and register callback
void setup() {
	Serial.begin();

	// Configure LED pin as output
	pinMode(LED1_PIN, OUTPUT);

	// Set interrupt handler
	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	if (!attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), blink, RISING)) {
		Serial.println("Setting button interrupt failed");
	}

}

// Do nothing in the loop
void loop() {

}
