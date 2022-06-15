#include <Mesh.h>

#define ROLE_RECEIVER
// #define ROLE_TRANSMITTER

Mesh mesh;

uint32_t counter;

void setup() {
	Serial.begin();
	counter = 0;
}

void loop() {

	// Serial.println("Loop");

#ifdef ROLE_RECEIVER
	// Read Mesh
	if (mesh.available()) {
		// Serial.println("Mesh message available");
		uint8_t* msg_ptr = nullptr;
		uint8_t stone_id = 0;
		uint8_t size = mesh.readMeshMsg(&msg_ptr, &stone_id);

		// Serial.print("Mesh msg from stone ");
		// Serial.print((int)stone_id);
		// Serial.print(" [");
		// Serial.print((int)size);
		// Serial.print("]: ");
		int i = 0;
		while (size != 0) {
			Serial.print(*(msg_ptr+i));
			size--;
		}
		Serial.println("");
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
		uint8_t msg[2] = {2, 3};
		uint8_t stoneId = 0; // broadcast
		mesh.sendMeshMsg(msg, sizeof(msg), stoneId);
	}
#endif
	counter++;
}

