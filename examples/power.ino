#include <Arduino.h>
#include <PowerUsage.h>

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Power example");
}

void loop() {
	int powerUsage = PowerUsage.getPowerUsageMilliWatts();
	Serial.print("Power usage is ");
	Serial.print(powerUsage);
	Serial.println(" mW");

	delay(1000);
}
