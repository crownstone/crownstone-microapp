#include <Arduino.h>
#include <Presence.h>

Presence presence;

void setup() {
	Serial.begin();

}

void loop() {

	// Check if profile X is present in room X.
	// roomID 0 in this case is the whole sphere.
	uint8_t profileId = 0;
	uint8_t roomId = 0;
	int isPresent = presence.isPresent(profileId, roomId);
	Serial.print("Is user in sphere: ");
	Serial.println(isPresent);
}

