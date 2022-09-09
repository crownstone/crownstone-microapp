#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A microapp example for acting as a BLE peripheral
 *
 * The example can easily be adjusted based on your own BLE central device.
 * Simply change the beaconAddress or beaconName constants.
 * You can adapt the scan handler onScannedDevice to match your wishes.
 */

// This is the value to be used in the temperature characteristic
uint8_t temperatureValue = 20;

void updateTemperature() {
	if (temperatureValue < 30) {
		temperatureValue++;
	}
	else {
		temperatureValue = 15;
	}
	temperatureCharacteristic.writeValue(temperatureValue);
}

// See Bluetooth SIG assigned numbers for the 16-bit UUIDs
// 0x181A: Environmental Sensing
BleService temperatureService("181A");
// 0x2A1C: Temperature measurement
BleByteCharacteristic temperatureCharacteristic("2A1C", BleCharacteristicProperties::BLERead);

void onCentralConnected(BleDevice device) {
	Serial.println("BLE central connected");
}

void onCentralDisconnected(BleDevice device) {
	Serial.println("BLE central disconnected");
}

// The Arduino setup function.
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("BLE scanner example");

	// Register scan handler
	if (!BLE.setEventHandler(BLEConnected, onCentralConnected)) {
		Serial.println("Setting event handler failed");
	}
	if (!BLE.setEventHandler(BLEDisconnected, onCentralDisconnected)) {
		Serial.println("Setting event handler failed");
	}
	temperatureService.addCharacteristic(temperatureCharacteristic);
	BLE.addService(temperatureService);
	temperatureCharacteristic.writeValue(temperatureValue);
}

// The Arduino loop function.
void loop() {
	updateTemperature();

	BleDevice central = BLE.central();

	if (central) {
		Serial.println("Central device connected");
		Serial.println(central.address());
	}
}
