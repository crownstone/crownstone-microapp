#include <CrownstoneSwitch.h>

/**
 * A basic microapp example showcasing dimmer functionality.
 *
 * This example should be run on a crownstone with dimmer functionality, and dimming enabled.
 * I.e. one connected to dimmable lights.
 * If run on one of the nRF52 development kits, the dimmer is mapped to one of the LEDs.
 */

uint8_t intensity = 0;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Dimming example");
}

void loop() {
	// Write new dimmer value
	CrownstoneSwitch.dim(intensity);

	// Increment the intensity until 100 percent, then reset.
	if (intensity >= 100) {
		intensity = 0;
	}
	else {
		intensity = intensity + 10;
	}

	delay(1000);
}
