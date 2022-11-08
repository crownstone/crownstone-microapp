#include <Arduino.h>

/**
 * A very basic 'hello world' microapp example.
 *
 * Every second a counter is incremented and printed.
 *
 * For the prints to work, the firmware needs to have UART and logs enabled.
 * You will also need a crownstone equipped with UART: a development kit or the crownstone USB dongle.
 */

// The loop counter.
unsigned int counter;

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown as bluenet info log).
	Serial.println("Hello world example");

	counter = 0;
}

// The Arduino loop function.
void loop() {
	// Print counter ever time we loop.
	Serial.println(counter++);

	// Make sure the loop is called at about once per second.
	delay(1000);
}
