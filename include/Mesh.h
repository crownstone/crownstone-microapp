#pragma once

#include <microapp.h>

class Mesh {
public:
	/**
	 * Send a mesh message.
	 *
	 * @param[in] msg         Pointer to the message.
	 * @param[in] msgSize     Size of the message, currently max 7.
	 * @param[in] stoneId     ID of the Crownstone to send the message to, or 0 to send it to every Crownstone.
	 */
	void sendMeshMsg(uint8_t* msg, uint8_t msgSize, uint8_t stoneId);
	
	/**
	 * Read a mesh message.
	 *
	 * @param[out] msg        Pointer that will be set to point to the message.
	 * @param[out] msgSize    Pointer that will be set to point to the stone ID of the sender.
	 * @return                Size of the message.
	 */
	uint8_t readMeshMsg(uint8_t** msg, uint8_t* stoneId);

	/**
	 * Check if a new message is avalable to read with the readMeshMsg function.
	 *
	 * @return                True if new message available, False if not.
	 */
	bool available();
};
