#include <Arduino.h>
#include <PowerUsage.h>

PowerUsage powerUsage;

void setup() {
	Serial.begin();
}

void loop() {

	// Get Power usage and print it
	int32_t milli_watts = powerUsage.get_usage_in_milli_watt();
	Serial.print("Power usage microapp: ");
	Serial.println(milli_watts);
}

