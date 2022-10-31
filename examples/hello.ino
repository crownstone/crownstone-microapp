#include <Arduino.h>
#include <ServiceData.h>


/**
 * A very basic 'hello world' microapp example.
 *
 * Every loop (once per second) a counter is incremented and printed.
 * For crownstones equipped with UART, the serial prints are sent over UART,
 * e.g. one of the nRF52 development kits or the crownstone USB dongle.
 * Most 'real' crownstones do not have an available UART port.
 * Hence, the counter is also advertised over the service data,
 * i.e. broadcast over BLE.
 */


uint16_t counter;

const microapp_size_t serviceDataSize = 1;
// This uuid can be used to identify the microapp
const uint16_t serviceDataUuid = 0x1234;
uint8_t serviceData[serviceDataSize];

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Hello world example");

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
