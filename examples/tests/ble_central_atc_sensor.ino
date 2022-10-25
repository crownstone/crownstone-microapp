#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A microapp example with the crownstone as a BLE central
 */

const char* peripheralAddress = "A4:C1:38:9A:45:E3";
const char* peripheralName = "ATC_9A45E3";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice& device) {
	Serial.print("   Microapp scan callback for ");
	Serial.println(device.address().c_str());
}

void onConnect(BleDevice& device) {
	Serial.print("   Microapp connect callback for ");
	Serial.println(device.address().c_str());
}

void onNotification(BleDevice& device, BleCharacteristic& characteristic, uint8_t* data, uint16_t size) {
	Serial.print("   Microapp notification for ");
	Serial.println(characteristic.uuid());
}

// The Arduino setup function.
void setup() {
	Serial.println("   BLE central ATC example");

	if (!BLE.begin()) {
		Serial.println("   BLE.begin failed");
		return;
	}

	// Register scan handler
	if (!BLE.setEventHandler(BLEDeviceScanned, onScannedDevice)) {
		Serial.println("   Setting event handler failed");
	}
	if (!BLE.setEventHandler(BLEConnected, onConnect)) {
		Serial.println("   Setting event handler failed");
	}
	BLE.scanForAddress(peripheralAddress);
	Serial.println("   End of setup");
}

// The Arduino loop function.
void loop() {
	// Poll for scanned devices
	BleDevice& peripheral = BLE.available();

	if(!peripheral) {
		return;
	}

	Serial.println("   Peripheral available:");
	Serial.println(peripheral.address());
	// Try to connect
	if (!peripheral.connect(10000)) {
		Serial.println("   Connecting failed");
		return;
	}
	// We are looking for service with uuid 181A 'Environmental Sensing'
	if (!peripheral.discoverService("181A")) {
		Serial.println("   Service discovery failed");
		peripheral.disconnect();
		return;
	}
	// Print all found characteristics
	for (uint8_t i = 0; i < peripheral.characteristicCount(); i++) {
		Serial.println(peripheral.characteristic(i).uuid());
	}
	// Check for 2A1F 'Temperature Celsius' characteristic
	if (!peripheral.hasCharacteristic("2A1F")) {
		Serial.println("   No temperature char found");
		peripheral.disconnect();
		return;
	}
	BleCharacteristic& temperatureCharacteristic = peripheral.characteristic("2A1F");
	temperatureCharacteristic.setEventHandler(BLENotification, onNotification);
	// Subscribe to the characteristic so that we get notifications
	if (!temperatureCharacteristic.subscribe()) {
		Serial.println("   Subscribing to char failed");
		peripheral.disconnect();
		return;
	}
	uint8_t counter = 0;
	uint8_t buffer[2];
	while (peripheral.connected()) {
		// Poll locally for notifications
		if (temperatureCharacteristic.valueUpdated()) {
			// Read characteristic value into buffer
			if (!temperatureCharacteristic.readValue(buffer, 2)) {
				Serial.println("   Reading failed");
				peripheral.disconnect();
				return;
			}
			Serial.println(buffer, 2);
			// Disconnect after 10 notifications
			if (counter++ > 10) {
				Serial.println("   Attempting disconnect");
				peripheral.disconnect();
				return;
			}
		}
		// Without a delay, microapp may retain control too long and bluenet watchdog may trigger
		delay(MICROAPP_LOOP_INTERVAL_MS);
	}
	// Start scanning again
	BLE.scanForAddress(peripheralAddress);
}