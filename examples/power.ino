#include <Arduino.h>
#include <PowerUsage.h>

PowerUsage powerUsage;

void setup() {
	Serial.begin();
}

void loop() {
	int powerUsageNow = powerUsage.getPowerUsageMilliWatts();
	Serial.print("Power usage is ");
	Serial.print(powerUsageNow);
	Serial.println(" mW");
}