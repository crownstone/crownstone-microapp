#include <CrownstoneRelay.h>

/**
 * A basic microapp example showcasing relay functionality
 *
 * This example should be run on a crownstone with switching functionality
 * This example will not do anything if run on one of the nRF52 development kits.
 */

CrownstoneRelay relay;
bool relayState = HIGH;
uint8_t counter = 0;

void setup() {
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
