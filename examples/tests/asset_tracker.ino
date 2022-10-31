#include <Arduino.h>
#include <BluenetInternal.h>


/**
 * This example uses the 'BluenetInternal' class to track accepted asset reports
 * It does so by setting a handler for 'EVT_RECV_MESH_MSG' events in bluenet
 * Note that this can't be done with the Mesh class for the microapp,
 * since that class is meant specifically for mesh messages of type CS_MESH_MODEL_TYPE_MICROAPP
 */

// See 'cs_Types.h' in bluenet
uint16_t EVT_RECV_MESH_MSG = 421;

// See 'cs_MeshModelPackets.h' in bluenet
uint8_t MESH_MSG_TYPE_ASSET_INFO_ID = 30;

uint32_t MY_ASSET_ID = 13893336;

int8_t rssi = 127;
uint8_t crownId = 0;
bool receivedAssetReport = false;

void onBluenetEvent(uint16_t bluenetType, uint8_t* data, microapp_size_t size) {
	// Buffer 'data' is an object of the MeshMsgEvent class in bluenet
	// Let's manually extract data from it
	uint8_t msgType = data[0];
	if (msgType != MESH_MSG_TYPE_ASSET_INFO_ID) {
		return;
	}
	// The MesgMsgEvent object contains a pointer to the actual message,
	// which is of type cs_mesh_model_msg_asset_report_id_t
	// Again, let's manually get the relevant data
	uint8_t* msg = reinterpret_cast<uint8_t*>(data[4] | ((data[5] << 8) & 0xFF00) | ((data[6] << 16) & 0xFF0000) | ((data[7] << 24) & 0xFF000000));
	uint32_t assetId = msg[0] | ((msg[1] << 8) & 0xFF00) | ((msg[2] << 16) & 0xFF0000);
	if (assetId == MY_ASSET_ID) {
		crownId = data[12];
		rssi = (int8_t)msg[4];
		receivedAssetReport = true;
	}
}

// The Arduino setup function.
void setup() {
	Serial.println("Asset tracker example");

	// set event handler
	if (!BluenetInternal.setEventHandler(onBluenetEvent)) {
		Serial.println("Setting event handler failed");
		return;
	}
	// subscribe to asset events
	if (!BluenetInternal.subscribe(EVT_RECV_MESH_MSG)) {
		Serial.println("Subscribing failed");
		return;
	}
}

// The Arduino loop function.
void loop() {
	// Print (only the last) asset report if one was received
	if (receivedAssetReport) {
		Serial.println("Asset detected: ");
		Serial.println((unsigned int)crownId);
		Serial.println((int)rssi);
	}
	receivedAssetReport = false;
}
