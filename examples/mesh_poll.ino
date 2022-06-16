#include <Mesh.h>
#include <Serial.h>

#define ROLE_RECEIVER
// #define ROLE_TRANSMITTER

uint32_t counter;

void meshCallback(MeshMsg msg) {
	Serial.println("mesh callback");
}

void setup() {
	Serial.begin();
	counter = 0;
	MESH.begin();
	// mesh.setIncomingMeshMsgHandler(meshCallback);

}

void loop() {

	// Serial.println("Loop");

#ifdef ROLE_RECEIVER
	// Read Mesh
	if (MESH.available()) {
		// Serial.println("Mesh message available");
		MeshMsg msg;
		MESH.readMeshMsg(&msg);

		Serial.println("Received mesh message:");
		Serial.println(*(msg.dataPtr));

		// Serial.print("Mesh msg from stone ");
		// Serial.print((int)stone_id);
		// Serial.print(" [");
		// Serial.print((int)size);
		// Serial.print("]: ");
		// int i = 0;
		// while (size != 0) {
		// 	Serial.print(*(msg_ptr+i));
		// 	size--;
		// }
		// Serial.println("");
		// Serial.println((*msg_ptr) + *(msg_ptr+1));
		// if (size != 2) {
		// 	Serial.println("Incorrect size");
		// }
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

