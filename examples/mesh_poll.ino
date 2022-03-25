#include <Mesh.h>

Mesh mesh;

uint32_t counter;

void setup() {
	Serial.begin();
	counter = 0;
}

void loop() {
	
	Serial.println("Loop");

	// Read Mesh
	if (mesh.available()) {
		Serial.println("Mesh messages available");
		uint8_t* msg_ptr = nullptr;
		uint8_t stone_id = 0;
		uint8_t size = mesh.readMeshMsg(&msg_ptr, &stone_id);

		Serial.print("Stone id ");
		Serial.print((short)stone_id);
		Serial.print(" gave response [");
		Serial.print((short)size);
		Serial.print("]: '");
		while (size-- != 0) {
			Serial.print((short)*msg_ptr++);
		}
		Serial.println("'");
	}
	if (counter % 5 == 0) {
		// Send Mesh
		Serial.println("Sending mesh msg");
		uint8_t msg[7] = {5,5,5,5,5,5,5};
		uint8_t stoneId = 0;
		mesh.sendMeshMsg(msg, sizeof(msg), stoneId);
	}
	counter++;
}

