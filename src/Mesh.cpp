#include <Mesh.h>
#include <Serial.h>

int handleMeshInterrupt(void* buf) {
	if (buf == nullptr) {
		return CS_ACK_ERR_NOT_FOUND;
	}
	microapp_sdk_mesh_t* mesh = reinterpret_cast<microapp_sdk_mesh_t*>(buf);
	// The only type of mesh interrupts are for now incoming messages
	return Mesh.handleIncomingMeshMsg(mesh);
}

MeshClass::MeshClass() :	_registeredIncomingMeshMsgHandler(nullptr),
							_stoneId(0) {}

bool MeshClass::listen() {
	// Register soft interrupt locally
	interrupt_registration_t interrupt;
	interrupt.major = CS_MICROAPP_SDK_TYPE_MESH;
	interrupt.minor = CS_MICROAPP_SDK_MESH_READ;
	interrupt.interruptFunc = handleMeshInterrupt;
	int result = registerInterrupt(&interrupt);
	if (result != CS_ACK_SUCCESS) {
		// No empty interrupt slots available
		return false;
	}

	// Also send a command to bluenet that we want to listen to mesh
	uint8_t* payload = getOutgoingMessagePayload();
	microapp_sdk_mesh_t* mesh = (microapp_sdk_mesh_t*)(payload);
	mesh->header.ack          = CS_ACK_REQUEST;
	mesh->header.sdkType      = CS_MICROAPP_SDK_TYPE_MESH;
	mesh->type                = CS_MICROAPP_SDK_MESH_LISTEN;

	sendMessage();

	return (mesh->header.ack == CS_ACK_SUCCESS);
}

int MeshClass::handleIncomingMeshMsg(microapp_sdk_mesh_t* msg) {
	// If a handler is registered, we do not need to copy anything to the buffer,
	// since the handler will deal with it right away.
	// The microapp's softInterrupt handler has copied the msg to a localCopy
	// so there is no worry of overwriting the msg upon a bluenet roundtrip
	if (_registeredIncomingMeshMsgHandler != nullptr) {
		MeshMsg handlerMsg = MeshMsg(msg->stoneId, msg->data, msg->size);
		_registeredIncomingMeshMsgHandler(handlerMsg);
		return CS_ACK_SUCCESS;
	}
	// Add msg to buffer or discard if full
	// Q: is it not more logical to discard oldest?
	// that will lead to more memcpy calls however
	bool full = true;
	int i;
	for (i=0; i<MESH_MSG_BUFFER_LEN; i++) {
		if (!_incomingMeshMsgBuffer[i].filled) {
			full = false;
			break;
		}
	}
	if (full) {
		// discard message
		return CS_ACK_ERR_NO_SPACE;
	}
	MeshMsgBufferEntry& copy = _incomingMeshMsgBuffer[i];
	copy.stoneId = msg->stoneId;
	copy.dlen = msg->size;
	memcpy(copy.data, msg->data, msg->size);
	copy.filled = true;

	return 0;
}

void MeshClass::setIncomingMeshMsgHandler(void (*handler)(MeshMsg)) {
	_registeredIncomingMeshMsgHandler = handler;
}

bool MeshClass::available() {
	for (int i=0; i<MESH_MSG_BUFFER_LEN; i++) {
		if (_incomingMeshMsgBuffer[i].filled) {
			return true;
		}
	}
	return false;
}

void MeshClass::readMeshMsg(MeshMsg* msg) {
	for (int i=MESH_MSG_BUFFER_LEN-1; i>=0; i--) {
		if (_incomingMeshMsgBuffer[i].filled) {
			// copy message data to another location where it can't be overwritten by incoming messages
			// (do not check if _availableMeshMsg was already filled, just overwrite)
			_availableMeshMsg.filled = true;
			memcpy(_availableMeshMsg.data, _incomingMeshMsgBuffer[i].data, MICROAPP_MAX_MESH_MESSAGE_SIZE);
			// create a mesh message to return to the user
			*msg = MeshMsg(	_incomingMeshMsgBuffer[i].stoneId,
							_availableMeshMsg.data,
							_incomingMeshMsgBuffer[i].dlen);

			// free the incoming buffer entry
			_incomingMeshMsgBuffer[i].filled = false;
			return;
		}
	}
}

void MeshClass::sendMeshMsg(uint8_t* msg, uint8_t msgSize, uint8_t stoneId) {
	uint8_t* payload = getOutgoingMessagePayload();
	microapp_mesh_send_cmd_t* cmd = (microapp_mesh_send_cmd_t*)(payload);
	cmd->meshHeader.header.cmd    = CS_MICROAPP_COMMAND_MESH;
	cmd->meshHeader.opcode        = CS_MICROAPP_COMMAND_MESH_SEND;
	cmd->stoneId                  = stoneId;

	int msgSizeSent = msgSize;
	if (msgSize > MICROAPP_MAX_MESH_MESSAGE_SIZE) {
		msgSize = MICROAPP_MAX_MESH_MESSAGE_SIZE;
	}

	cmd->dlen = msgSizeSent;
	memcpy(cmd->data, msg, msgSizeSent);

	sendMessage();
}

short MeshClass::id() {
	// First check if we already cached the id before
	if (_stoneId != 0) {
		return _stoneId;
	}
	// If not, ask bluenet via a MESH_GET_INFO message
	uint8_t* payload = getOutgoingMessagePayload();
	microapp_mesh_info_cmd_t* cmd = (microapp_mesh_info_cmd_t*)(payload);
	cmd->meshHeader.header.cmd = CS_MICROAPP_COMMAND_MESH;
	cmd->meshHeader.header.ack = false;
	cmd->meshHeader.opcode = CS_MICROAPP_COMMAND_MESH_GET_INFO;
	sendMessage();

	if (cmd->meshHeader.header.ack) {
		_stoneId = cmd->stoneId;
	}
	return _stoneId;
}