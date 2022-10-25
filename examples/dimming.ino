#include <CrownstoneDimmer.h>

/**
 * A basic microapp example showcasing dimmer functionality
 *
 * This example should be run on a crownstone with dimmer functionality
 * I.e. one connected to dimmable lights
 * This example will not do anything if run on one of the nRF52 development kits.
 */

CrownstoneDimmer dimmer;
uint8_t intensity = 0;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Dimming example");
}

void loop() {
	// Write new dimmer value
	dimmer.setIntensity(intensity);

	// Increment the loop counter until 100 percent, then reset
	if (intensity >= 100) {
		intensity = 0;
	}
	else {
		intensity = intensity + 10;
	}
}
