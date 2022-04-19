//
// Dimmer demo
//

#include <Arduino.h>
#include <CrownstoneDimmer.h>

CrownstoneDimmer dimmer;
uint8_t intensity = 0;

void setup() {
	// Initialize the dimmer
	dimmer.init();
}

void loop() {
	// Write new dimmer value
	dimmer.setIntensity(intensity);

	// Increment the loop counter until 100 percent
	if (intensity >= 100) {
		intensity = 0;
	}
	else {
		intensity = intensity + 10;
	}
}
