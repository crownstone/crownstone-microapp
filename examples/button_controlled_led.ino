#include <Arduino.h>

/**
 * A basic microapp example showcasing interrupt functionality.
 *
 * Every time the button is pressed, the LED is toggled (from on to off, or off to on).
 *
 * This example should be run on a 'crownstone' equipped with buttons and LEDs,
 * e.g. one of the nRF52 development kits.
 * Most 'real' crownstones are not equipped with buttons and LEDs,
 * so this example will not do anything for these.
 */

// Which LED to use.
const uint8_t ledPin = LED1_PIN;

// Which button to use.
const uint8_t buttonPin = BUTTON2_PIN;

boolean state = LOW;

// Callback for a button press
void blink() {
	Serial.println("blink");

	// Toggle LED
	state = !state;
	digitalWrite(ledPin, state);
}

// In setup, configure button and LED GPIO pins and register callback
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Button controlled LED example");

	// Configure LED pin as output
	pinMode(ledPin, OUTPUT);

	// Set interrupt handler
	pinMode(buttonPin, INPUT_PULLUP);
	if (!attachInterrupt(digitalPinToInterrupt(buttonPin), blink, RISING)) {
		Serial.println("Setting button interrupt failed");
	}

}

// Do nothing in the loop
void loop() {

}
