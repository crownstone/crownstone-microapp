#include <Arduino.h>
#include <ServiceData.h>

// A microapp example that prints hello world.
uint16_t counter;

const size_t serviceDataSize = 1;
const uint16_t serviceDataUuid = 0x1234;
uint8_t serviceData[serviceDataSize];

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Hello world");

	counter = 0;
}

// The Arduino loop function.
void loop() {
	// Print counter ever time we loop (which is every second)
	Serial.println(counter++);

	// Also advertise the counter value in the service data.
	serviceData[0] = counter;
	ServiceData.write(serviceDataUuid, serviceData, serviceDataSize);
}
