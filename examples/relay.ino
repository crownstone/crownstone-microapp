#include <Arduino.h>
#include <CrownstoneRelay.h>

/**
 * A basic microapp example showcasing relay functionality
 *
 * This example should be run on a crownstone with switching functionality
 * If run on one of the nRF52 development kits, the relay is mapped to one of the LEDs
 */

CrownstoneRelay relay;
bool relayState = HIGH;
uint8_t counter = 0;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Relay example");
}

void loop() {
	if (counter++ % 10 != 0) {
		return;
	}
	// Toggle the relay
	// relay.switchToggle() can be used alternatively
	if (relayState) {
		relay.switchOff();
	}
	else {
		relay.switchOn();
	}
	relayState = !relayState;
}
