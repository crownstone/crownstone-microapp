#include <ArduinoBLE.h>

// A ble microapp example for reading advertisements from a Xiaomi Thermometer with custom firmware: https://github.com/atc1441/ATC_MiThermometer

uint16_t loopCounter = 0;

bool scanToggle = false;

const char* myAddress = "A4:C1:38:9A:45:E3";
//const char* myName = "ATC_9A45E3";
//const char* myUuid = "181A";

// For more extensive information about the scanned device
#define PRINT_EXTENSIVE
#undef PRINT_EXTENSIVE

// callback for received peripheral advertisement
void onScannedDevice(BleDevice device) {

	Serial.println("BLE device scanned");
	Serial.print("\trssi: ");
	Serial.println(device.rssi());

	Serial.print("\taddress: ");
	Serial.println(device.address().c_str());

#ifdef PRINT_EXTENSIVE
	if (device.hasLocalName()) {
		Serial.print("\tComplete local name: ");
		Serial.println(device.localName().c_str());
	}
#endif

	// parse service data of Xiaomi device advertisement if available
	data_ptr_t serviceData;
	if (device.findAdvertisementDataType(GapAdvType::ServiceData, &serviceData)) {
		if (serviceData.len == 15) { // service data length of the Xiaomi service data advertisements
			uint16_t temperature = (serviceData.data[8] << 8) | serviceData.data[9];
			Serial.print("\tTemperature: "); Serial.println(temperature);
#ifdef PRINT_EXTENSIVE
			uint8_t humidity = serviceData.data[10];
			Serial.print("\tHumidity: "); Serial.println(humidity);
			uint16_t battery_perc = serviceData.data[11];
			Serial.print("\tBattery \%: "); Serial.println(battery_perc);
#endif
		}
		else {
			Serial.println("\tIncorrect service data length");
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
	Serial.println(loopCounter);

	// we would like to loop every 10000 ms (10 seconds)
	if ((loopCounter >= (10000 / MICROAPP_LOOP_INTERVAL_MS)))
	{
		scanToggle = !scanToggle;
		Serial.println("Toggle scanning");
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
		loopCounter = 0;
	}
	loopCounter++;
}
