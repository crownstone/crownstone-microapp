#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A test microapp for writable characteristics with the crownstone as peripheral
 */

BleService writableService;
static const uint8_t NR_WRITABLE_BYTES = 2;
uint8_t writableValue[NR_WRITABLE_BYTES];
BleCharacteristic writableCharacteristic;

// To be called when the characteristic is written by a central device
void onCharacteristicWritten(BleDevice device, BleCharacteristic characteristic) {
	Serial.println("Characteristic written callback");
}

// The Arduino setup function.
void setup() {
	Serial.begin();
	Serial.println("BLE peripheral writable example");

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}
	writableService = BleService("12340000-ABCD-1234-5678-ABCDEF123456");
	writableCharacteristic = BleCharacteristic("12340001-ABCD-1234-5678-ABCDEF123456",
		BleCharacteristicProperties::BLERead | BleCharacteristicProperties::BLEWrite,
		writableValue, NR_WRITABLE_BYTES);

	// Register handler
	writableCharacteristic.setEventHandler(BLEWritten, onCharacteristicWritten);
	// Add characteristic to service and service to BLE (order is important)
	writableService.addCharacteristic(writableCharacteristic);
	BLE.addService(writableService);
	writableCharacteristic.writeValue(writableValue, NR_WRITABLE_BYTES);
}

// The Arduino loop function.
void loop() {
	if (writableCharacteristic.written()) {
		Serial.println(writableCharacteristic.value(), writableCharacteristic.valueLength());
	}

	BleDevice& central = BLE.central();
	if (central) {
		// Keep the connection alive
		central.connectionKeepAlive();
	}
}
