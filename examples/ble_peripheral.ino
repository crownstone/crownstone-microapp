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
static const uint8_t NR_TEMPERATURE_BYTES = 2;
uint8_t temperatureValue[NR_TEMPERATURE_BYTES] = {20, 25};
// See Bluetooth SIG assigned numbers for the 16-bit UUIDs
// 0x181A: Environmental Sensing
BleService temperatureService("181A");
// 0x2A1C: Temperature measurement
BleCharacteristic temperatureCharacteristic("2A1C", BleCharacteristicProperties::BLERead);


void updateTemperature() {
	// Simulate some temperature changes
	for (int i = 0; i < NR_TEMPERATURE_BYTES; i++){
		if (temperatureValue[i] < 30) {
			temperatureValue[i]++;
		}
		else {
			temperatureValue[i] = 15;
		}
	}
	temperatureCharacteristic.writeValue(temperatureValue, NR_TEMPERATURE_BYTES);
}

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

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}

	// Register scan handler
	if (!BLE.setEventHandler(BLEConnected, onCentralConnected)) {
		Serial.println("Setting event handler failed");
		return;
	}
	if (!BLE.setEventHandler(BLEDisconnected, onCentralDisconnected)) {
		Serial.println("Setting event handler failed");
		return;
	}
	temperatureService.addCharacteristic(temperatureCharacteristic);
	BLE.addService(temperatureService);
	temperatureCharacteristic.writeValue(temperatureValue, NR_TEMPERATURE_BYTES);
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
