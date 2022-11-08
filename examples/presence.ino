#include <Arduino.h>
#include <Presence.h>

/**
 * Turns a LED on when someone with a given profile ID is in the given room ID.
 *
 * This example should be run on a 'crownstone' equipped with LEDs,
 * e.g. one of the nRF52 development kits
 * Most 'real' crownstones are not equipped with LEDs,
 * so this example will not do anything for these.
 */

// Which profile to use.
// A profile is a group of users. 0 indicates all users.
const uint8_t profileId = 0;

// Which room to use.
// 0 is a special case indicating the entire sphere (any room).
const uint8_t roomId = 0;

// Which LED to use.
// LEDs 3 and 4 are reserved for relay and dimmer on PCA10040 boards.
const uint8_t ledPin = LED1_PIN;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Presence example");

	// Set LED to OUTPUT, so we can write.
	pinMode(ledPin, OUTPUT);
}

void loop() {
	bool isInRoom = Presence.isPresent(profileId, roomId);
	if (isInRoom) {
		Serial.println("Profile is in sphere");

		// Turn the LED on.
		digitalWrite(ledPin, true);
	}
	else {
		Serial.println("Profile is not in sphere");

		// Turn the LED off.
		digitalWrite(ledPin, false);
	}
}
