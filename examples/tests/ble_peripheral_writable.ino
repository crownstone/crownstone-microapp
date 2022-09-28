#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A test microapp for writable characteristics with the crownstone as peripheral
 */

BleService writableService;
static const uint8_t NR_WRITABLE_BYTES = 2;
uint8_t writableValue[NR_WRITABLE_BYTES];
BleCharacteristic writableCharacteristic;

void onCharacteristicWritten(BleDevice device, BleCharacteristic characteristic) {
	Serial.println("Characteristic written callback");
}

// The Arduino setup function.
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("BLE peripheral example");

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}
	writableService = BleService("BEBE");
	// custom uuids are also possible
	const char writableCharacteristicUuid[] = "12345678-ABCD-1234-5678-ABCDEF123456";
	writableCharacteristic = BleCharacteristic(writableCharacteristicUuid,
		BleCharacteristicProperties::BLERead | BleCharacteristicProperties::BLEWrite,
		writableValue, NR_WRITABLE_BYTES);

	// Register handler
	writableCharacteristic.setEventHandler(BLEWritten, onCharacteristicWritten);
	writableService.addCharacteristic(writableCharacteristic);
	BLE.addService(writableService);
	writableCharacteristic.writeValue(writableValue, NR_WRITABLE_BYTES);
}

// The Arduino loop function.
void loop() {
	if (writableCharacteristic.written()) {
		Serial.println(writableCharacteristic.value(), writableCharacteristic.valueLength());
	}
}
