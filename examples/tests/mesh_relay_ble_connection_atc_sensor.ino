#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Mesh.h>

/**
 * A microapp example that relays a BLE connection over the mesh.
 *
 * Roughly like so:
 * [sensor] <--connection--> [crownstone] <--mesh--> [crownstone] <--connection--> [phone]
 */

// The name of the sensor.
const char* sensorName = "ATC_9A45E3";

// The service UUID that has the characteristic we relay.
const char* serviceUuid        = "181A";

// The characteristic UUID we relay.
const char* characteristicUuid = "2A1F";

// The characteristic value we relay.
uint16_t myCharacteristicValue = 0;

// Keep up whether we are connected as central to the sensor.
bool connectedToSensor = false;

// Keep up the device we are connected to.
BleDevice* connectedDevice = nullptr;

// Create the service and characteristic.
BleService myService(serviceUuid);
BleCharacteristic myCharacteristic(
		characteristicUuid,
		BleCharacteristicProperties::BLERead | BleCharacteristicProperties::BLENotify,
		(uint8_t*)&myCharacteristicValue,
		sizeof(myCharacteristicValue));

void onConnect(BleDevice& device) {
	connectedDevice = &device;
	BLE.stopScan();
}

void onDisconnect(BleDevice& device) {
	connectedToSensor = false;
	connectedDevice = nullptr;
	digitalWrite(LED1_PIN, 0);
	digitalWrite(LED2_PIN, 0);
}

void meshCallback(MeshMsg msg) {
	Serial.print("   Received mesh message:");
	Serial.println(msg.dataPtr, msg.size);

	if (msg.size > sizeof(myCharacteristicValue)) {
		return;
	}

	if (!connectedToSensor && connectedDevice != nullptr) {
		Serial.println("   Update characteristic value");
		memcpy((uint8_t*)&myCharacteristicValue, msg.dataPtr, msg.size);
		// Instead of directly writing the characteristic, a flag could be set
		// which is read in the loop, and only then write the characteristic
		// That would throttle the amount of notifications on the phone
		// which for some applications can be beneficial
		BleCharacteristic& characteristic = connectedDevice->characteristic(characteristicUuid);
		characteristic.writeValue((uint8_t*)&myCharacteristicValue, msg.size);
	}
}

void setup() {
	Serial.begin();
	Serial.println("   BLE connection relay example");

	pinMode(LED1_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);
	digitalWrite(LED1_PIN, 0);
	digitalWrite(LED2_PIN, 0);


	if (!BLE.begin()) {
		Serial.println("   BLE.begin failed");
		return;
	}

	// Always first add characteristics to the service, then add the service.
	myService.addCharacteristic(myCharacteristic);
	BLE.addService(myService);

	myCharacteristic.writeValue((uint8_t*)&myCharacteristicValue, sizeof(myCharacteristicValue));

	// Set event handlers last, so we don't interrupt the setup.
	Mesh.setIncomingMeshMsgHandler(meshCallback);
	if (!Mesh.listen()) {
		Serial.println("   Mesh.listen failed");
	}

	BLE.setEventHandler(BleEventType::BLEConnected, onConnect);
	BLE.setEventHandler(BleEventType::BLEDisconnected, onDisconnect);

	Serial.println("   End of setup");
}

// Returns true on successful connect.
bool connect(BleDevice& peripheral) {
	Serial.println("   Connect to the sensor");
	if (!peripheral.connect()) {
		Serial.println("   Failed to connect");
		return false;
	}

	if (!peripheral.discoverService(serviceUuid)) {
		Serial.println("   Discovery failed");
		return false;
	}

	if (!peripheral.hasCharacteristic(characteristicUuid)) {
		Serial.println("   Characteristic missing");
		peripheral.disconnect();
		return false;
	}
	BleCharacteristic& characteristic = peripheral.characteristic(characteristicUuid);
	if (!characteristic.subscribe()) {
		Serial.println("   Failed to subscribe");
		peripheral.disconnect();
		return false;
	}
	return true;
}



void loop() {
	if (!BLE.connected()) {
		// Try to connect to a scanned sensor, or start scanning for a sensor.
		if (connect(BLE.available())) {
			connectedToSensor = true;
			digitalWrite(LED1_PIN, 1);
		}
		else {
			BLE.scanForName(sensorName);
		}
		return;
	}

	if (connectedToSensor) {
		BleCharacteristic& characteristic = connectedDevice->characteristic(characteristicUuid);
		if (!characteristic.valueUpdated()) {
			return;
		}
		uint8_t value[sizeof(myCharacteristicValue)];
		if (!characteristic.readValue(value, sizeof(value))) {
			return;
		}

		Serial.println("   Received notification, send mesh message");
		Mesh.sendMeshMsg(value, sizeof(value), 0);
		return;
	}

	// Connected to phone
	BleDevice& central = BLE.central();
	if (central) {
		digitalWrite(LED2_PIN, 1);
		// Keep the connection alive.
		central.connectionKeepAlive();
	}

}
