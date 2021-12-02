#include <ArduinoBLE.h>

// A ble microapp example for reading advertisements from a Xiaomi Thermometer with custom firmware: https://github.com/atc1441/ATC_MiThermometer
uint16_t counter = 0;
bool scanToggle = true;
const char* myAddress = "A4:C1:38:9A:45:E3";
const char* myName = "ATC_9A45E3";
const char* myUuid = "181A";

void my_callback_func(microapp_ble_dev_t dev) // callback for received peripheral advertisement
{
	Serial.println("my_callback_func: ");
	Serial.print("\trssi: "); Serial.println(dev.rssi);
	Serial.print("\taddress type: "); Serial.println(dev.addr_type);
	Serial.print("\taddress: "); Serial.println(dev.addr,sizeof(dev.addr));
	Serial.print("\tdlen: "); Serial.println(dev.dlen);
	Serial.print("\tdata: "); Serial.println(dev.data,dev.dlen);

	// parse advertisement data
	uint8_t adType = dev.data[1];
	switch(adType) {
		case 0x09 : { // complete local name
			Serial.print("\tComplete local name: ");
			char* localName = (char*) &(dev.data[2]);
			Serial.println(localName,dev.dlen-2);
			break;
		}
		case 0x16 : { // service data
			Serial.println("\tService data");
			uint8_t UUID[2] = {dev.data[3], dev.data[2]};
			Serial.print("\t\tUUID: "); Serial.println(UUID,2);
			uint16_t temperature = (dev.data[10] << 8) | dev.data[11];
			Serial.print("\t\tTemperature: "); Serial.println(temperature);
			uint8_t humidity = dev.data[12];
			Serial.print("\t\tHumidity: "); Serial.println(humidity);
			uint16_t battery_perc = dev.data[13];
			Serial.print("\t\tBattery \%: "); Serial.println(battery_perc);
		}
	}
}

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Setup");

	// Register my_callback_func
	BLE.setEventHandler(BleEventDeviceScanned, my_callback_func);
}

// The Arduino loop function.
int loop() {

	// Say something every time we loop (which is every second)
	Serial.println("Loop");

	if (scanToggle)
	{
		BLE.scanForAddress(myAddress);
		// BLE.scanForName(myName);
		// BLE.scanForUuid(myUuid);
	}
	else
	{
		BLE.stopScan();
	}

	counter++;
	if ((counter % 10) == 0) // every 10 loops, toggle scanning
	{
		scanToggle = !scanToggle;
	}
	return 1;

}