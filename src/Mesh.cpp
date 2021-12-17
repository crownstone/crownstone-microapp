#include <Mesh.h>

static const uint8_t CMD_SIZE = 1;
uint8_t mesh_header_size = sizeof(microapp_mesh_header_t);

void Mesh :: sendMeshMsg(uint8_t * msg, uint8_t msg_size, uint8_t stoneId){

	uint8_t mesh_header_send_size =  sizeof(microapp_mesh_send_header_t);

	global_msg.payload[0] = CS_MICROAPP_COMMAND_MESH;

	microapp_mesh_header_t *send_mesh_cmd_opcode =
		(microapp_mesh_header_t*)&global_msg.payload[CMD_SIZE];
	send_mesh_cmd_opcode->opcode = CS_MICROAPP_COMMAND_MESH_SEND;

	microapp_mesh_send_header_t *send_mesh_cmd_header=
		(microapp_mesh_send_header_t*)&global_msg.payload[CMD_SIZE + mesh_header_size];
	send_mesh_cmd_header->stoneId = stoneId;

	memcpy(&global_msg.payload[CMD_SIZE + mesh_header_size + mesh_header_send_size], msg, msg_size);

	global_msg.length = CMD_SIZE + mesh_header_size + mesh_header_send_size + msg_size;
	sendMessage(&global_msg);
}

uint8_t Mesh :: readMeshMsg(uint8_t ** msg_ptr, uint8_t * stone_id_ptr){

	uint8_t mesh_header_read_size = sizeof(microapp_mesh_read_t);

	global_msg.payload[0] = CS_MICROAPP_COMMAND_MESH;
	microapp_mesh_header_t *read_mesh_cmd_opcode =
		(microapp_mesh_header_t*)&global_msg.payload[CMD_SIZE];
	read_mesh_cmd_opcode->opcode = CS_MICROAPP_COMMAND_MESH_READ;

	microapp_mesh_read_t *read_mesh_cmd_header=
		(microapp_mesh_read_t*)&global_msg.payload[CMD_SIZE + mesh_header_size];
	read_mesh_cmd_header->messageSize = 0;


	global_msg.length = CMD_SIZE + mesh_header_size + mesh_header_read_size;
	sendMessage(&global_msg);

	*stone_id_ptr = read_mesh_cmd_header->stoneId;
	*msg_ptr = read_mesh_cmd_header->message;

	return read_mesh_cmd_header->messageSize;
}

bool Mesh :: available(){

	uint8_t is_available_header_size = sizeof(microapp_mesh_read_available_t);

	global_msg.payload[0] = CS_MICROAPP_COMMAND_MESH;

	microapp_mesh_header_t *is_available_cmd_opcode =
		(microapp_mesh_header_t*)&global_msg.payload[CMD_SIZE];
	is_available_cmd_opcode->opcode = CS_MICROAPP_COMMAND_MESH_READ_AVAILABLE;

	microapp_mesh_read_available_t *is_available_cmd_header=
		(microapp_mesh_read_available_t*)&global_msg.payload[CMD_SIZE + mesh_header_size];
	is_available_cmd_header->available = false;

	global_msg.length = CMD_SIZE + mesh_header_size + is_available_header_size;
	sendMessage(&global_msg);

	return is_available_cmd_header->available;
}
