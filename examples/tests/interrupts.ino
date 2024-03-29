#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Mesh.h>

/**
 * A basic microapp example showcasing interrupts.
 *
 * The example can configure three types of interrupts:
 * - BLE device scans
 * - Received mesh messages
 * - Button presses
 * All three can be used at the same time.
 * On interrupts, LEDs are toggled (for nRF52 development kits),
 * and messages are printed to UART
 */

#define ENABLE_BLE_INTERRUPTS
#define ENABLE_MESH_INTERRUPTS
#define ENABLE_GPIO_INTERRUPTS

boolean led1state = LOW;
boolean led2state = LOW;
boolean led3state = LOW;

const char* bleBeaconAddress = "A4:C1:38:9A:45:E3";

#ifdef ENABLE_BLE_INTERRUPTS
void onScannedDevice(BleDevice& device) {
	// toggle led1
	Serial.println("Scanned device");
	led1state = !led1state;
	digitalWrite(LED1_PIN, led1state);
}
#endif

#ifdef ENABLE_MESH_INTERRUPTS
void onReceivedMeshMsg(MeshMsg msg) {
	// toggle led2
	Serial.println("Received mesh message");
	led2state = !led2state;
	digitalWrite(LED2_PIN, led2state);
}
#endif

#ifdef ENABLE_GPIO_INTERRUPTS
void onPushedButton() {
	// toggle led3
	Serial.println("Detected button press");
	led3state = !led3state;
	digitalWrite(LED3_PIN, led3state);
}
#endif

void setup() {
	Serial.println("Interrupt test");

	// initialize gpio
	pinMode(LED1_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);
	pinMode(LED3_PIN, OUTPUT);

#ifdef ENABLE_GPIO_INTERRUPTS
	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	if (!attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), onPushedButton, RISING)) {
		Serial.println("Setting button interrupt failed");
	}
#endif

#ifdef ENABLE_BLE_INTERRUPTS
	// initialize ble
	if (!BLE.setEventHandler(BLEDeviceScanned, onScannedDevice)) {
		Serial.println("Setting BLE event handler failed");
	}
	if (!BLE.scanForAddress(bleBeaconAddress)) {
		Serial.println("Scanning for BLE address failed");
	}
#endif

#ifdef ENABLE_MESH_INTERRUPTS
	// initialize mesh
	Mesh.setIncomingMeshMsgHandler(onReceivedMeshMsg);
	if (!Mesh.listen()) {
		Serial.println("Listening to mesh failed");
	}
#endif
}

void loop() {

}