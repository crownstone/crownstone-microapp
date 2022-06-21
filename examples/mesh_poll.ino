#include <Mesh.h>
#include <Serial.h>

#define ROLE_RECEIVER
// #define ROLE_TRANSMITTER

uint32_t counter;

void printMeshMsg(MeshMsg* msg) {
	Serial.print("Received mesh message from stone ");
	Serial.println(msg->stoneId);
	for (int i = 0; i < msg->dlen; i++) {
		Serial.print(*(msg->dataPtr + i));
		if (i + 1 == msg->dlen) {
			Serial.println(" ");
		}
		else {
			Serial.print(" ");
		}
	}
}

void meshCallback(MeshMsg msg) {
	printMeshMsg(&msg);
}

void setup() {
	Serial.begin();
	counter = 0;

#ifdef ROLE_RECEIVER
	Serial.println("Start listening to mesh");
	if (!MESH.listen()) {
		Serial.println("Mesh.listen() failed");
	}
	MESH.setIncomingMeshMsgHandler(meshCallback);
#endif
}

void loop() {
#ifdef ROLE_RECEIVER
	// Read Mesh
	if (MESH.available()) {
		MeshMsg msg;
		MESH.readMeshMsg(&msg);
		printMeshMsg(&msg);
	}
#endif
#ifdef ROLE_TRANSMITTER
	if (counter % 10 == 0) {
		// Send Mesh
		Serial.println("Sending mesh msg");
		uint8_t msg[2] = {22, 21};
		uint8_t stoneId = 0; // broadcast
		MESH.sendMeshMsg(msg, sizeof(msg), stoneId);
	}
#endif
	counter++;
}

