#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Mesh.h>

/**
 * An advanced microapp example showcasing communication between two crownstones
 * This microapp has to run on at least two crownstones
 * The eventual goal is to let a crownstone LED blink via a button press on another crownstone
 * The example showcases mesh and bluetooth functionality
 * The flow is as follows:
 * - During setup, the crownstones start advertising a custom service
 * - On a button press, a crownstone broadcasts a mesh request
 * - On a received mesh request, the app replies with its own address
 * - On a received mesh reply, the app starts to scan for the provided address
 * - On a scan match, a connection is set up and discovery is performed
 * - The central crownstone reads the characteristic value, increments all bytes by one, and writes it back
 * - On a write event, the peripheral will set the LED to the first characteristic byte modulo 2 (i.e. toggle it)
 */

enum MeshHeader {
	MESH_REQUEST = 1,
	MESH_REPLY = 2,
};

bool buttonPressed;

bool meshRequestReceived;
uint8_t meshRequestStoneId;
bool meshReplyReceived;
uint8_t meshReplyAddress[6];

// The characteristic uses the same base UUID as the service.
const char* serviceUuid        = "ABCD0000-1234-ABCD-1234-4321FFFF1234";
const char* characteristicUuid = "ABCD0001-1234-ABCD-1234-4321FFFF1234";
uint8_t myCharacteristicValue[37];

BleService myService(serviceUuid);
BleCharacteristic myCharacteristic(
		characteristicUuid,
		BleCharacteristicProperties::BLEWrite | BleCharacteristicProperties::BLERead | BleCharacteristicProperties::BLENotify,
		myCharacteristicValue,
		sizeof(myCharacteristicValue));

// callback for received peripheral advertisement
void onScannedDevice(BleDevice device) {
	Serial.println("   Microapp scan callback:");
	Serial.println(device.address().c_str());
}

void onConnect(BleDevice device) {
	Serial.println("   Microapp connect callback:");
	Serial.println(device.address().c_str());
}

void onButtonPress() {
	buttonPressed = true;
}

void meshCallback(MeshMsg msg) {
	Serial.print("   Received mesh message from stone ");
	Serial.println(msg.stoneId);
	Serial.println(msg.dataPtr, msg.size);

	if (msg.size >= 1 && msg.dataPtr[0] == MESH_REQUEST) {
		meshRequestReceived = true;
		meshRequestStoneId = msg.stoneId;
	}
	if (msg.size >= 7 && msg.dataPtr[0] == MESH_REPLY) {
		meshReplyReceived = true;
		memcpy(meshReplyAddress, msg.dataPtr + 1, 6);
	}
}

void onWrite(BleDevice& device, BleCharacteristic& characteristic) {
	if (!characteristic.readValue(myCharacteristicValue, sizeof(myCharacteristicValue))) {
		return;
	}

	digitalWrite(LED1_PIN, myCharacteristicValue[0] % 2);
}

void setup() {
	Serial.begin();
	Serial.println("   Remote blinky example");

	pinMode(LED1_PIN, OUTPUT);

	pinMode(BUTTON1_PIN, INPUT_PULLUP);
	buttonPressed = false;

	meshRequestReceived = false;
	meshReplyReceived = false;

	if (!BLE.begin()) {
		Serial.println("BLE.begin failed");
		return;
	}

	// Always first add characteristics to the service, then add the service.
	myService.addCharacteristic(myCharacteristic);
	BLE.addService(myService);

	// Set the initial characteristic value.
	for (size_t i = 0; i < sizeof(myCharacteristicValue); ++i) {
		myCharacteristicValue[i] = i;
	}
	myCharacteristic.writeValue(myCharacteristicValue, sizeof(myCharacteristicValue));

	// Set event handlers last, so we don't interrupt the setup.
	Mesh.setIncomingMeshMsgHandler(meshCallback);
	if (!Mesh.listen()) {
		Serial.println("Mesh.listen failed");
	}

	myCharacteristic.setEventHandler(BleEventType::BLEWritten, onWrite);

	if (!attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), onButtonPress, FALLING)) {
		Serial.println("Setting button interrupt failed");
	}

	Serial.println("   End of setup");
}

void loop() {
	if (buttonPressed) {
		uint8_t msg[1];
		msg[0] = MESH_REQUEST;
		Mesh.sendMeshMsg(msg, sizeof(msg), 0);
		Serial.println("Mesh request sent");
		buttonPressed = false;
		return;
	}

	if (meshRequestReceived) {
		meshRequestReceived = false;
		uint8_t msg[7];
		msg[0] = MESH_REPLY;

		MacAddress myAddress(BLE.address().c_str());
		const uint8_t* myAddressBytes = myAddress.bytes();
		if (myAddressBytes == nullptr) {
			return;
		}
		memcpy(msg + 1, myAddressBytes, 6);
		Mesh.sendMeshMsg(msg, sizeof(msg), meshRequestStoneId);
	}

	if (meshReplyReceived) {
		meshReplyReceived = false;
		MacAddress remoteAddress(meshReplyAddress, 6, 0);
		Serial.println(remoteAddress.string());
		BLE.scanForAddress(remoteAddress.string());
		return;
	}

	// If we are scanning, and the address is found, connect to it.
	BleDevice& peripheral = BLE.available();
	if (peripheral) {
		Serial.println("   Connecting..");
		if (!peripheral.connect()) {
			Serial.println("   Connect failed");
			return;
		}

		BLE.stopScan();

		if (!peripheral.discoverService(serviceUuid)) {
			Serial.println("   Discovery failed");
			peripheral.disconnect();
			return;
		}

		if (!peripheral.hasCharacteristic("0001")) {
			Serial.println("   Characteristic not found");
			peripheral.disconnect();
			return;
		}

		BleCharacteristic& characteristic = peripheral.characteristic("0001");

		// Read the current value, increase each byte with 1, then write that.
		uint8_t characteristicValue[sizeof(myCharacteristicValue)];
		if (!characteristic.readValue(characteristicValue, sizeof(characteristicValue))) {
			Serial.println("   Failed to read value");
			peripheral.disconnect();
			return;
		}

		for (size_t i = 0; i < sizeof(characteristicValue); ++i) {
			characteristicValue[i] += 1;
		}

		if (!characteristic.writeValue(characteristicValue, sizeof(characteristicValue))) {
			Serial.println("   Failed to write value");
			peripheral.disconnect();
			return;
		}

		Serial.println("   Written value");
		peripheral.disconnect();
		return;
	}
}