#include <Arduino.h>
#include <PowerUsage.h>

PowerUsage powerUsage;

void setup() {
	Serial.begin();
	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Power example");
}

void loop() {
	int powerUsageNow = powerUsage.getPowerUsageMilliWatts();
	Serial.print("Power usage is ");
	Serial.print(powerUsageNow);
	Serial.println(" mW");
}