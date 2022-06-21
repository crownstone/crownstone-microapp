#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Mesh.h>
#include <Serial.h>

boolean led1state = LOW;
boolean led2state = LOW;
boolean led3state = LOW;

const char* bleBeaconAddress = "A4:C1:38:9A:45:E3";

void onScannedDevice(BleDevice device) {
	// toggle led1
	Serial.println("Scanned device");
	led1state = !led1state;
	digitalWrite(LED1_PIN, led1state);
}

void onReceivedMeshMsg(MeshMsg msg) {
	// toggle led2
	Serial.println("Received mesh message");
	led2state = !led2state;
	digitalWrite(LED2_PIN, led2state);
}

void onPushedButton() {
	// toggle led3
	Serial.println("Detected button press");
	led3state = !led3state;
	digitalWrite(LED3_PIN, led3state);
}

void setup() {
	Serial.begin();
	Serial.println("Interrupt test code");

	// initialize gpio
	pinMode(LED1_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);
	pinMode(LED3_PIN, OUTPUT);
	pinMode(BUTTON1_PIN, INPUT_PULLUP);
	if (attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), onPushedButton, RISING) < 0) {
		Serial.println("Setting button interrupt failed");
	}

	delay(1500);

	// initialize ble
	if (!BLE.setEventHandler(BleEventDeviceScanned, onScannedDevice)) {
		Serial.println("Setting BLE event handler failed");
	}
	if (!BLE.scanForAddress(bleBeaconAddress)) {
		Serial.println("Scanning for BLE address failed");
	}

	// initialize mesh
	MESH.setIncomingMeshMsgHandler(onReceivedMeshMsg);
	if (!MESH.listen()) {
		Serial.println("Listening to mesh failed");
	}
}

void loop() {

}