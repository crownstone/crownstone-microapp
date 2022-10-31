#include <Arduino.h>

/**
 * A very basic 'hello world' microapp example.
 *
 * Every loop (once per second) a counter is incremented and printed.
 * For crownstones equipped with UART, the serial prints are sent over UART,
 * e.g. one of the nRF52 development kits or the crownstone USB dongle.
 * Most 'real' crownstones do not have an available UART port.
 * Hence, the counter is also advertised over the service data,
 * i.e. broadcast over BLE.
 */


uint16_t counter;


// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Hello world example");

	counter = 0;
}

// The Arduino loop function.
void loop() {
	// Print counter ever time we loop (which is every second)
	Serial.println(counter++);

	delay(1000);
}
