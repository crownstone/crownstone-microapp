#include <Mesh.h>

int softInterruptMesh(void* args, void* buf) {
	microapp_mesh_read_cmd_t* msg = (microapp_mesh_read_cmd_t*)buf;
	return MESH.handleIncomingMeshMsg(msg);
}

Mesh::Mesh() {
	for (int i=0; i<MESH_MSG_BUFFER_LEN; i++) {
		_incomingMeshMsgBuffer[i].filled = false;
	}
	_hasRegisteredIncomingMeshMsgHandler = false;
	_registeredIncomingMeshMsgHandler = nullptr;
}

int Mesh::handleIncomingMeshMsg(microapp_mesh_read_cmd_t* msg) {
	// If a handler is registered, we do not need to copy anything to the buffer,
	// since the handler will deal with it right away.
	// The microapp's softInterrupt handler has copied the msg to a localCopy
	// so there is no worry of overwriting the msg upon a bluenet roundtrip
	if (_hasRegisteredIncomingMeshMsgHandler) {
		MeshMsg handlerMsg = MeshMsg(msg->stoneId, msg->data, msg->dlen);
		_registeredIncomingMeshMsgHandler(&handlerMsg);
		return 0;
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
		return -1;
	}
	MeshMsgBufferEntry& copy = _incomingMeshMsgBuffer[i];
	copy.stoneId = msg->stoneId;
	copy.dlen = msg->dlen;
	memcpy(copy.data, msg->data, msg->dlen);
	copy.filled = true;

	return 0;
}

void Mesh::setIncomingMeshMsgHandler(void (*handler)(MeshMsg*)) {
	_hasRegisteredIncomingMeshMsgHandler = true;
	_registeredIncomingMeshMsgHandler = handler;
}

MeshMsg* Mesh::readMeshMsg() {
	for (int i=MESH_MSG_BUFFER_LEN-1; i>=0; i--) {
		if (_incomingMeshMsgBuffer[i].filled) {
			_incomingMeshMsgBuffer[i].filled = false;
			MeshMsg msg = MeshMsg(	_incomingMeshMsgBuffer[i].stoneId,
									_incomingMeshMsgBuffer[i].data,
									_incomingMeshMsgBuffer[i].dlen);
			return &msg;
		}
	}
}

///////////////////////////////////////////////////////////////////
// old

void Mesh::sendMeshMsg(uint8_t* msg, uint8_t msgSize, uint8_t stoneId) {
	uint8_t* payload = getOutgoingMessagePayload();
	microapp_mesh_send_cmd_t* cmd = (microapp_mesh_send_cmd_t*)(payload);
	cmd->mesh_header.header.cmd   = CS_MICROAPP_COMMAND_MESH;
	cmd->mesh_header.opcode       = CS_MICROAPP_COMMAND_MESH_SEND;
	cmd->stoneId                  = stoneId;

	int msgSizeSent = msgSize;
	if (msgSize > MICROAPP_MAX_MESH_MESSAGE_SIZE) {
		msgSize = MICROAPP_MAX_MESH_MESSAGE_SIZE;
	}

	cmd->dlen = msgSizeSent;
	memcpy(cmd->data, msg, msgSizeSent);

	sendMessage();
}

uint8_t Mesh::readMeshMsg(uint8_t** msgPtr, uint8_t* stoneIdPtr) {
	uint8_t* payloadOut              = getOutgoingMessagePayload();
	microapp_mesh_read_cmd_t* cmdOut = (microapp_mesh_read_cmd_t*)(payloadOut);
	cmdOut->mesh_header.header.cmd   = CS_MICROAPP_COMMAND_MESH;
	cmdOut->mesh_header.opcode       = CS_MICROAPP_COMMAND_MESH_READ;

	sendMessage();

	// Actually the mesh message is written into the wrong buffer by bluenet...
	microapp_mesh_read_cmd_t* cmdIn = cmdOut;

	// uint8_t* payloadIn              = getIncomingMessagePayload();
	// microapp_mesh_read_cmd_t* cmdIn = (microapp_mesh_read_cmd_t*)(payloadIn);

	*stoneIdPtr = cmdIn->stoneId;
	*msgPtr     = cmdIn->data;

	return cmdIn->dlen;
}

bool Mesh::available() {
	uint8_t* payload = getOutgoingMessagePayload();
	microapp_mesh_read_available_cmd_t* cmd = (microapp_mesh_read_available_cmd_t*)(payload);
	cmd->mesh_header.header.cmd             = CS_MICROAPP_COMMAND_MESH;
	cmd->mesh_header.opcode                 = CS_MICROAPP_COMMAND_MESH_READ_AVAILABLE;

	sendMessage();

	return cmd->available;
}
