#include <Arduino.h>
#include <Presence.h>
#include <Mesh.h>
#include <PowerUsage.h>

PowerUsage powerUsage;
Presence presence;
Mesh mesh;

void setup() {
	Serial.begin();

}

void loop() {

	// Read Mesh
	if(mesh.available()){
		uint8_t * msg_ptr = nullptr;
		uint8_t stone_id = 0;
		uint8_t size = mesh.readMeshMsg(&msg_ptr, &stone_id); 

		Serial.print("Stone Id: ");
		Serial.print((short)stone_id);
		Serial.print(" gave responce : ");

		while(size-- != 0){
			Serial.print((short)*msg_ptr++);
		}
		Serial.println("");
	}

	// Send Mesh
	// uint8_t msg[7] = {1,2,3,4,5,6,7};
	uint8_t msg[7] = {5,5,5,5,5,5,5};
	uint8_t stoneId = 0;
	mesh.sendMeshMsg(msg, sizeof(msg), stoneId);

	// Power usage
	int32_t milli_watts = powerUsage.get_usage_in_milli_watt();
	Serial.print("Power usage microapp: ");
	Serial.println(milli_watts);

	// Presence
	uint8_t profileId = 0;
	uint8_t roomId = 0;
	int isPresent = presence.isPresent(profileId, roomId);
	Serial.print("Is user in sphere: ");
	Serial.println(isPresent);
}

