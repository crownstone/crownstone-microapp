#pragma once

#include <microapp.h>

#define MESH_MSG_BUFFER_LEN 3

typedef void (*ReceivedMeshMsgHandler)(MeshMsg);

struct MeshMsgBufferEntry {
	bool filled;
	uint8_t stoneId;
	uint8_t data[MICROAPP_MAX_MESH_MESSAGE_SIZE];
	uint8_t dlen;
};

/**
 * Wrapper class for mesh message
 * This class itself contains only pointers to the actual mesh message data,
 * which for read mesh messages is stored in the mesh buffer
 * and for sent messages is stored on the user side.
 * Since the actual data is stored somewhere else, the validity of a MeshMsg object
 * is only guaranteed for the lifetime of the actual message data.
 */
class MeshMsg {
private:
	MeshMsg(){};

	uint8_t _stoneId;
	uint8_t* _dataPtr;
	uint8_t _dlen;

public:
	MeshMsg(uint8_t stoneId, uint8_t* dataPtr, uint8_t dlen) {
		_stoneId = stoneId;
		_dataPtr = dataPtr;
		_dlen = dlen;
	}
}

// wrapper for class method
int softInterruptMesh(void* args, void* buf);

/**
 * Mesh class for inter-crownstone messaging.
 * There are two main methods of reading mesh messages:
 * - Polling (via available() and readMeshMessage())
 * - Interrupts (via setIncomingMeshMsgHandler())
 * Sending mesh messages is supported via sendMeshMsg()
 */
class Mesh {
private:
	/**
	 * Constructor
	 */
	Mesh();

	/**
	 * Buffer for storing incoming mesh messages
	 * This is where the actual data is stored (for polling applications)
	 * Note: A MeshMsgBufferEntry is max 10 bytes in size
	 */
	MeshMsgBufferEntry _incomingMeshMsgBuffer[MESH_MSG_BUFFER_LEN];

	// If true, call the handler function registered by the user upon incoming mesh messages
	bool _hasRegisteredIncomingMeshMsgHandler;

	ReceivedMeshMsgHandler _registeredIncomingMeshMsgHandler;

public:

	static Mesh & getInstance() {
		// Guaranteed to be destroyed.
		static Mesh instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Place a mesh message
	 */
	int handleIncomingMeshMsg(microapp_mesh_read_cmd_t* msg);

	/**
	 * Register a handler that will be called upon incoming mesh messages
	 */
	void setIncomingMeshMsgHandler(void (*handler)(MeshMsg*));

	/**
	 * Pop a message from the incoming mesh message buffer.
	 * Note that the returned pointer is not memory safe.
	 * It should only be used in local context.
	 * If the message content needs to be saved, the caller
	 * is responsible for copying the data to some memory-safe location.
	 *
	 * @return Pointer to a message
	 */
	MeshMsg* readMeshMsg();

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
	 * @param[out] stoneId    Pointer that will be set to point to the stone ID of the sender.
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

#define MESH Mesh::getInstance()