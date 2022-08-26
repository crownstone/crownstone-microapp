#include <Arduino.h>
#include <Presence.h>

const uint8_t profileId = 0; // a profile is a group of users. 0 indicates all users
const uint8_t roomId = 0; // 0 is a special case indicating the entire sphere

Presence presence;

void setup() {
	Serial.begin();
}

void loop() {
	bool isInRoom = presence.isPresent(profileId, roomId);
	if (isInRoom) {
		Serial.println("Profile is in sphere");
	}
	else {
		Serial.println("Profile is not in sphere");
	}
}