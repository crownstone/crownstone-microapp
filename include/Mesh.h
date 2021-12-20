#pragma once

#include <microapp.h>

class Mesh {
public:
	void sendMeshMsg(uint8_t* msg, uint8_t msg_size, uint8_t stoneId);
	uint8_t readMeshMsg(uint8_t** msg_ptr, uint8_t* stone_id_ptr);

	// Check if message available to read
	bool available();
};
