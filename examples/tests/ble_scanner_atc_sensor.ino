#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A microapp example for scanning BLE advertisements.
 * The beacon used is a Xiaomi Thermometer with custom firmware:
 * https://github.com/atc1441/ATC_MiThermometer
 *
 * The example can easily be adjusted based on your own BLE peripheral.
 * Simply change the beaconAddress or beaconName constants.
 * You can adapt the scan handler onScannedDevice to match your wishes.
 */

uint16_t loopCounter = 0;
bool scanToggle = false;

const char* beaconAddress = "A4:C1:38:9A:45:E3";
const char* beaconName = "ATC_9A45E3";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice& device) {
	if (device.hasLocalName()) {
		Serial.println(device.localName().c_str());
		return;
	}
	Serial.println(device.address().c_str());
	Serial.println(device.rssi());

	// parse service data of beacon advertisement if available
	ble_ad_t serviceData;
	if (device.findAdvertisementDataType(GapAdvType::ServiceData16BitUuid, &serviceData)) {
		if (serviceData.len == 15) { // service data length of the ATC service data advertisements
			uint16_t temperature = (serviceData.data[8] << 8) | serviceData.data[9];
			Serial.println(temperature);
		}
	}
}

// The Arduino setup function.
void setup() {
	Serial.println("BLE scanner ATC example");

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}

	// Register scan handler
	if (!BLE.setEventHandler(BLEDeviceScanned, onScannedDevice)) {
		Serial.println("Setting event handler failed");
	}
}

// The Arduino loop function.
void loop() {

	// Say something every time we loop (which is every second)
	Serial.println(loopCounter);

	// we would like to loop every 10000 ms (10 seconds)
	if ((loopCounter >= 10))
	{
		scanToggle = !scanToggle;
		if (scanToggle)
		{
			Serial.println("Start scanning");
			BLE.scanForAddress(beaconAddress);
			// BLE.scanForName(beaconName); // Alternative
		}
		else
		{
			Serial.println("Stop scanning");
			BLE.stopScan();
		}
		loopCounter = 0;
	}
	loopCounter++;
	// wait a second
	delay(1000);
}
