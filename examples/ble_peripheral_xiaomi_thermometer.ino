#include <ArduinoBLE.h>

// A ble microapp example for reading advertisements from a Xiaomi Thermometer with custom firmware: https://github.com/atc1441/ATC_MiThermometer

static uint16_t counter = 1;

bool scanToggle = false;

const char* myAddress = "A4:C1:38:9A:45:E3";
//const char* myName = "ATC_9A45E3";
//const char* myUuid = "181A";

// const char* myAddress = "DC:9F:FE:40:F3:1B";
// const char* myName = "CRWN";
// const char* myUuid = "181A";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice device) {

	Serial.println("BLE device scanned");
	Serial.print("\trssi: ");
	Serial.println(device.rssi());

	Serial.print("\taddress: ");
	Serial.println(device.address().c_str());

	if (device.hasLocalName()) {
		Serial.print("\tComplete local name: ");
		Serial.println(device.localName().c_str());
	}

	// parse service data of Xiaomi device advertisement if available
	data_ptr_t serviceData;
	if (device.findAdvertisementDataType(GapAdvType::ServiceData, &serviceData)) {
		if (serviceData.len == 15) { // service data length of the Xiaomi service data advertisements
			Serial.println("\tService data");
			uint8_t UUID[2] = {serviceData.data[1], serviceData.data[0]};
			Serial.print("\t\tUUID: "); Serial.println(UUID,2);
			uint16_t temperature = (serviceData.data[8] << 8) | serviceData.data[9];
			Serial.print("\t\tTemperature: "); Serial.println(temperature);
			uint8_t humidity = serviceData.data[10];
			Serial.print("\t\tHumidity: "); Serial.println(humidity);
			uint16_t battery_perc = serviceData.data[11];
			Serial.print("\t\tBattery \%: "); Serial.println(battery_perc);
		}
		else {
			Serial.println("\tIncorrect thermometer service data length");
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
	BLE.setEventHandler(BleEventDeviceScanned, onScannedDevice);
}

// The Arduino loop function.
void loop() {

	// Say something every time we loop (which is every second)
	Serial.println(counter);

	// we would like to loop every 10000 ms (10 seconds)
	if ((counter >= (10000 / MICROAPP_LOOP_INTERVAL_MS)))
	{
		scanToggle = !scanToggle;
		Serial.println("Toggle");
		if (scanToggle)
		{
			// BLE.scan(); // unfiltered!
			BLE.scanForAddress(myAddress);
			// BLE.scanForName(myName);
			// BLE.scanForUuid(myUuid);
		}
		else
		{
			BLE.stopScan();
		}
		counter = 0;
	}
	counter++;
}
