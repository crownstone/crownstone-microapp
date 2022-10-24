#include <Arduino.h>
#include <ArduinoBLE.h>

/**
 * A microapp example for connecting to BLE peripherals
 * The peripheral device used for this app is a Nordic Thingy:52
 * https://www.nordicsemi.com/thingy
 *
 * The example can easily be adjusted based on your own BLE peripheral
 */

const char* peripheralName = "thingy";

// callback for received peripheral advertisement
void onScannedDevice(BleDevice& thingy) {
	Serial.print("   Scanned thingy with address ");
	Serial.println(thingy.address().c_str());
}

void onConnect(BleDevice& thingy) {
	Serial.print("   Connected to thingy with address ");
	Serial.println(thingy.address().c_str());
}

void onNotification(BleDevice& thingy, BleCharacteristic& characteristic, uint8_t* data, uint16_t size) {
	Serial.print("   Received notification for ");
	Serial.println(characteristic.uuid());
	Serial.println(data, size);
}

// The Arduino setup function.
void setup() {
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("   BLE central thingy example");

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
	BLE.scanForName(peripheralName);
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
	// We are looking for service with specific uuid
	const char* thingyUserInterfaceServiceUuid = "ef680300-9b35-4933-9b10-52ffa9740042";
	if (!peripheral.discoverService(thingyUserInterfaceServiceUuid)) {
		Serial.println("   Service discovery failed");
		peripheral.disconnect();
		return;
	}
	// Print all found characteristics
	for (uint8_t i = 0; i < peripheral.characteristicCount(); i++) {
		Serial.println(peripheral.characteristic(i).uuid());
	}
	const char* thingyLEDCharacteristicUuid = "ef680301-9b35-4933-9b10-52ffa9740042";
	if (!peripheral.hasCharacteristic(thingyLEDCharacteristicUuid)) {
		Serial.println("   No LED char found");
		peripheral.disconnect();
		return;
	}
	BleCharacteristic& ledCharacteristic = peripheral.characteristic(thingyLEDCharacteristicUuid);
	const char* thingyButtonCharacteristicUuid = "ef680302-9b35-4933-9b10-52ffa9740042";
	if (!peripheral.hasCharacteristic(thingyButtonCharacteristicUuid)) {
		Serial.println("   No button char found");
		peripheral.disconnect();
		return;
	}
	BleCharacteristic& buttonCharacteristic = peripheral.characteristic(thingyButtonCharacteristicUuid);
	buttonCharacteristic.setEventHandler(BLENotification, onNotification);
	// Subscribe to the button characteristic so that we get notifications
	if (!buttonCharacteristic.subscribe()) {
		Serial.println("   Subscribing to button char failed");
		peripheral.disconnect();
		return;
	}
	uint8_t counter = 0;
	uint8_t ledRGB[4] = {0x01, 0x00, 0x00, 0x00};
	while (peripheral.connected()) {
		// every iteration, make led either red, green or blue
		ledRGB[1] = ((counter + 0) % 3 == 0) * 0xFF;
		ledRGB[2] = ((counter + 1) % 3 == 0) * 0xFF;
		ledRGB[3] = ((counter + 2) % 3 == 0) * 0xFF;
		ledCharacteristic.writeValue(ledRGB, 4);
		if (counter++ > 10) {
			Serial.println("   Attempting to disconnect");
			peripheral.disconnect();
			return;
		}
		delay(1000);
	}
	// Start scanning again
	BLE.scanForName(peripheralName);
}