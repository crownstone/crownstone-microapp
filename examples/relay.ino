#include <Arduino.h>
#include <CrownstoneSwitch.h>

/**
 * A basic microapp example showcasing relay functionality.
 *
 * This example should be run on a Crownstone with a relay, and dimming disabled.
 * If run on one of the nRF52 development kits, with debug firmware, the relay is mapped to one of the LEDs.
 */

uint8_t counter = 0;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Relay example");
}

void loop() {
	// Toggle the relay
	// Of course, you could also use CrownstoneSwitch.toggle() instead.
	if (CrownstoneSwitch.isOn()) {
		CrownstoneSwitch.turnOff();
	}
	else {
		CrownstoneSwitch.turnOn();
	}

	delay(3000);
}
