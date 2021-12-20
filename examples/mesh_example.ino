#include <Arduino.h>
#include <Mesh.h>

Mesh mesh;

void setup() {
	Serial.begin();
}

void loop() {

	// Read Mesh
	if (mesh.available()) {
		uint8_t * msg_ptr = nullptr;
		uint8_t stone_id = 0;
		uint8_t size = mesh.readMeshMsg(&msg_ptr, &stone_id); 

		Serial.print("Stone Id: ");
		Serial.print((short)stone_id);
		Serial.print(" gave responce : ");

		while (size-- != 0) {
			Serial.print((short)*msg_ptr++);
		}
		Serial.println("");
	}

	// Send Mesh
	uint8_t msg[7] = {5,5,5,5,5,5,5};
	uint8_t stoneId = 0;
	mesh.sendMeshMsg(msg, sizeof(msg), stoneId);
}

