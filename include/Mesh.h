#pragma once

#include <microapp.h>

#define MESH_MSG_BUFFER_LEN 2

struct MeshMsgBufferEntry {
	bool filled = false;
	uint8_t stoneId;
	uint8_t data[MAX_MICROAPP_MESH_PAYLOAD_SIZE];
	uint8_t size;
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
public:
	uint8_t stoneId;
	uint8_t* dataPtr;
	uint8_t size;

	MeshMsg(){};
	MeshMsg(uint8_t stoneId_, uint8_t* dataPtr_, uint8_t size_) {
		stoneId = stoneId_;
		dataPtr = dataPtr_;
		size = size_;
	};
};

typedef void (*ReceivedMeshMsgHandler)(MeshMsg);

microapp_result_t handleMeshInterrupt(void* buf);

/**
 * Mesh class for inter-crownstone messaging.
 * There are two main methods of reading mesh messages:
 * - Polling (via available() and readMeshMessage())
 * - Interrupts (via setIncomingMeshMsgHandler())
 * Sending mesh messages is supported via sendMeshMsg()
 */
class MeshClass {
private:
	/**
	 * Allow c wrapper to call private member functions
	 */
	friend microapp_result_t handleMeshInterrupt(void*);

	/**
	 * Constructors and copy constructors
	 */
	MeshClass();
	MeshClass(MeshClass const&);
	void operator=(MeshClass const&);

	/**
	 * Buffer for storing incoming mesh messages
	 * This is where the actual data is stored (for polling applications)
	 */
	MeshMsgBufferEntry _incomingMeshMsgBuffer[MESH_MSG_BUFFER_LEN];

	/**
	 * Available mesh message passed to the user
	 * Requires a separate memory location than _incomingMeshMsgBuffer since those can be overwritten
	 */
	MeshMsgBufferEntry _availableMeshMsg;

	/**
	 * Handler for registered callbacks for incoming mesh messages
	 */
	ReceivedMeshMsgHandler _registeredIncomingMeshMsgHandler;

	/**
	 * Own stone id
	 */
	uint8_t _stoneId;

	/**
	 * Handle an incoming mesh message. Called by c wrapper function
	 */
	microapp_result_t handleIncomingMeshMsg(microapp_sdk_mesh_t* msg);

public:

	static MeshClass & getInstance() {
		// Guaranteed to be destroyed.
		static MeshClass instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Initialize the mesh object for reading. Bluenet will start
	 * forwarding microapp mesh messages after calling this function.
	 *
	 * @return true if bluenet successfully registered an interrupt service routine
	 * @return false if registering an interrupt service routine failed
	 */
	bool listen();

	/**
	 * Register a handler that will be called upon incoming mesh messages
	 */
	void setIncomingMeshMsgHandler(ReceivedMeshMsgHandler handler);

	/**
	 * Check if a new message is avalable to read with the readMeshMsg function.
	 *
	 * @return                True if new message available, False if not.
	 */
	bool available();

	/**
	 * Read a mesh message
	 * Pop a message from the incoming mesh message buffer.
	 * Note that the returned pointer is not memory safe.
	 * It should only be used in local context.
	 * If the message content needs to be saved, the caller
	 * is responsible for copying the data to some memory-safe location.
	 *
	 * @param[in] msg   Pointer to the message.
	 */
	void readMeshMsg(MeshMsg* msg);

	/**
	 * Send a mesh message.
	 *
	 * @param[in] msg         Pointer to the message.
	 * @param[in] msgSize     Size of the message, currently max 7.
	 * @param[in] stoneId     ID of the Crownstone to send the message to, or 0 to send it to every Crownstone.
	 */
	void sendMeshMsg(uint8_t* msg, uint8_t msgSize, uint8_t stoneId);

	/**
	 * Get own stone id
	 *
	 * If already asked bluenet before, won't ask again but use cached value
	 */
	short id();
};

#define Mesh MeshClass::getInstance()