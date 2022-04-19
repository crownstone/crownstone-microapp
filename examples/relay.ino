//
// Relay demo
//

#include <Arduino.h>
#include <CrownstoneRelay.h>

CrownstoneRelay relay;
bool relayState = HIGH;
uint8_t counter = 0;

void setup() {
	// Initialize the relay
	relay.init();
}

void loop() {
	if (counter++ % 10 != 0) {
		return;
	}
	// Toggle the relay
	if (relayState) {
		relay.switchOff();
	}
	else {
		relay.switchOn();
	}
	relayState = !relayState;
}
