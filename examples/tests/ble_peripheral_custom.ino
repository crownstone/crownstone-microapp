#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A test microapp for writable characteristics with the crownstone as peripheral
 */

uint32_t loopCounter = 0;

BleService customService;

static const uint8_t NR_WRITABLE_BYTES = 20;
uint8_t writableValue[NR_WRITABLE_BYTES];
BleCharacteristic writableCharacteristic;

static const uint8_t NR_READABLE_BYTES = 20;
uint8_t readableValue[NR_READABLE_BYTES];
BleCharacteristic readableCharacteristic;

// To be called when the characteristic is written by a central device
void onCharacteristicSubscribed(BleDevice device, BleCharacteristic characteristic) {
	Serial.println("Characteristic subscribed callback");
}

// The Arduino setup function.
void setup() {
	Serial.begin();
	Serial.println("BLE peripheral custom service example");

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}
	customService = BleService("12340000-ABCD-1234-5678-ABCDEF123456");
	writableCharacteristic = BleCharacteristic("12340001-ABCD-1234-5678-ABCDEF123456",
		BleCharacteristicProperties::BLEWrite,
		writableValue, NR_WRITABLE_BYTES);
	readableCharacteristic = BleCharacteristic("12340002-ABCD-1234-5678-ABCDEF123456",
		BleCharacteristicProperties::BLERead | BleCharacteristicProperties::BLENotify,
		readableValue, NR_READABLE_BYTES);

	// Register handler
	readableCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
	// Add characteristics to service and service to BLE
	customService.addCharacteristic(writableCharacteristic);
	customService.addCharacteristic(readableCharacteristic);
	BLE.addService(customService);
}

// The Arduino loop function.
void loop() {

	loopCounter++;
	readableValue[0] = loopCounter & 0xFF;
	readableValue[1] = (loopCounter >> 8) & 0xFF;
	readableValue[2] = (loopCounter >> 16) & 0xFF;
	readableValue[3] = (loopCounter >> 24) & 0xFF;
	readableCharacteristic.writeValue(readableValue, 4);

	if (writableCharacteristic.written()) {
		Serial.println(writableCharacteristic.value(), writableCharacteristic.valueLength());
	}
	uint8_t len = writableCharacteristic.valueLength();
	uint8_t loopBuffer[len];
	writableCharacteristic.readValue(loopBuffer, len);
	Serial.println(loopBuffer, len);

	BleDevice& central = BLE.central();
	if (central) {
		// Keep the connection alive
		central.connectionKeepAlive();
	}
}
