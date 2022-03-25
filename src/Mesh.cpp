#include <Mesh.h>

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
