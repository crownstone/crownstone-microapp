#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A microapp example with the crownstone as a BLE central
 */

const char* peripheralAddress = "A4:C1:38:9A:45:E3";
// const char* peripheralName = "ATC_9A45E3";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice& device) {
	Serial.println("   Microapp scan callback:");
	Serial.println(device.address().c_str());
}

void onConnect(BleDevice& device) {
	Serial.println("   Microapp connect callback:");
	Serial.println(device.address().c_str());
}

// The Arduino setup function.
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("   BLE central example");

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}

	// Register scan handler
	if (!BLE.setEventHandler(BLEDeviceScanned, onScannedDevice)) {
		Serial.println("   Setting event handler failed");
	}
	if (!BLE.setEventHandler(BLEConnected, onConnect)) {
		Serial.println("   Setting event handler failed");
	}
	// Find devices with 'Environmental Sensing' service
	BLE.scanForAddress(peripheralAddress);
	Serial.println("   End of setup");
}

// The Arduino loop function.
void loop() {
	BleDevice& peripheral = BLE.available();

	if(!peripheral) {
		return;
	}

	Serial.println("   Peripheral available:");
	Serial.println(peripheral.address());
	if (!peripheral.connect(10000)) {
		Serial.println("   Connecting failed");
		return;
	}
	if (!peripheral.discoverService("181A")) {
		Serial.println("   Service discovery failed");
		peripheral.disconnect();
		return;
	}
	// Check for 'Temperature Celsius' characteristic
	if (!peripheral.hasCharacteristic("2A1F")) {
		Serial.println("   No temperature char found");
		peripheral.disconnect();
		return;
	}
	BleCharacteristic& temperatureCharacteristic = peripheral.characteristic("2A1F");
	uint8_t counter = 0;
	while (peripheral.connected()) {
		uint8_t buffer[2];
		temperatureCharacteristic.readValue(buffer, 2);
		Serial.println(buffer, 2);
		delay(1000);
		if (counter++ > 10) {
			Serial.println("   Attempting disconnect");
			peripheral.disconnect();
			return;
		}
	}
	BLE.scanForAddress(peripheralAddress);
}