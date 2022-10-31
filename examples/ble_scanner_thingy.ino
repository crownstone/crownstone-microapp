#include <Arduino.h>
#include <ArduinoBLE.h>
#include <BleUuid.h>

/**
 * A microapp example for scanning BLE advertisements.
 * The peripheral device used for this app is a Nordic Thingy:52
 * https://www.nordicsemi.com/thingy
 *
 * The example can easily be adjusted based on your own BLE peripheral.
 */

uint16_t loopCounter = 0;
bool scanToggle = false;

// This is the localName that is advertised by the device
// Scanned devices will be filtered based on it
const char* thingyName = "thingy";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice& device) {

	Serial.println("Device scanned: ");
	Serial.println(device.address().c_str());
	if (device.hasLocalName()) {
		Serial.println(device.localName().c_str());
	}
	// parse list of 128-bit service uuids that should be advertised
	ble_ad_t serviceData;
	if (device.findAdvertisementDataType(GapAdvType::IncompleteList128BitServiceUuids, &serviceData)) {
		for (uint8_t i = 0; i < serviceData.len; i += UUID_128BIT_BYTE_LENGTH) {
			Serial.println(Uuid(serviceData.data, UUID_128BIT_BYTE_LENGTH).string());
		}
	}
}

// The Arduino setup function.
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("BLE scanner thingy example");

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
	if ((loopCounter++ > 10))
	{
		scanToggle = !scanToggle;
		if (scanToggle)
		{
			Serial.println("Start scanning");
			BLE.scanForName(thingyName);
		}
		else
		{
			Serial.println("Stop scanning");
			BLE.stopScan();
		}
		loopCounter = 0;
	}
	delay(1000);
}
