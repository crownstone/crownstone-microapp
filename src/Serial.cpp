#include <Serial.h>

#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

// Design choice is that strings will always be null-terminated.
// The last byte will be overwritten at the bluenet side by a null byte even if this is not done in the microapp code.

void SerialBase_::begin() {
	// does nothing
}

int SerialBase_::write(char value) {
	return _write(value);
}

int SerialBase_::write(float value) {
	return _write(value);
}

int SerialBase_::write(double value) {
	return _write(value);
}

int SerialBase_::write(int value) {
	return _write(value);
}

int SerialBase_::write(short value) {
	return _write(value);
}

int SerialBase_::write(unsigned int value) {
	return _write(value);
}

int SerialBase_::write(const char *str) {
	return _write(str);
}

int SerialBase_::write(String str, int length) {
	return _write(str, length);
}

int SerialBase_::write(const uint8_t *buf, int length) {
	return _write(buf, length);
}

// Copies of the above functions

int SerialBase_::print(char value) {
	return _write(value);
}

int SerialBase_::print(float value) {
	return _write(value);
}

int SerialBase_::print(double value) {
	return _write(value);
}

int SerialBase_::print(int value) {
	return _write(value);
}

int SerialBase_::print(short value) {
	return _write(value);
}

int SerialBase_::print(unsigned int value) {
	return _write(value);
}

int SerialBase_::print(const char *str) {
	return _write(str);
}

int SerialBase_::print(String str, int length) {
	return _write(str, length);
}

int SerialBase_::print(const uint8_t *buf, int length) {
	return _write(buf, length);
}

// Copies of the above functions, but with newline option enabled

int SerialBase_::println(char value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(float value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(double value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(int value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(short value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(unsigned int value) {
	return _write(value, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(const char *str) {
	return _write(str, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(String str, int length) {
	return _write(str, length, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

int SerialBase_::println(const uint8_t *buf, int length) {
	return _write(buf, length, CS_MICROAPP_COMMAND_LOG_NEWLINE);
}

/// Implementations (protected)

int SerialBase_::_write(char value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_char_cmd_t* command = reinterpret_cast<microapp_log_char_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::Char, option);
}

int SerialBase_::_write(float value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_float_cmd_t* command = reinterpret_cast<microapp_log_float_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::Float, option);
}

int SerialBase_::_write(double value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_double_cmd_t* command = reinterpret_cast<microapp_log_double_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::Double, option);
}

int SerialBase_::_write(int value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_int_cmd_t* command = reinterpret_cast<microapp_log_int_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::Int, option);
}

int SerialBase_::_write(short value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_short_cmd_t* command = reinterpret_cast<microapp_log_short_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::Short, option);
}

int SerialBase_::_write(unsigned int value, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_int_cmd_t* command = reinterpret_cast<microapp_log_int_cmd_t*>(payload);
	command->value = value;
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = sizeof(value);
	return _write(cmd, Type::UnsignedInt, option);
}

int SerialBase_::_write(const char *str, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_string_cmd_t* command = reinterpret_cast<microapp_log_string_cmd_t*>(payload);
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = strlen(str);
	memcpy(command->str, str, cmd->length);
	return _write(cmd, Type::Str, option);
}

int SerialBase_::_write(String str, int length, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_string_cmd_t* command = reinterpret_cast<microapp_log_string_cmd_t*>(payload);
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = strlen(str.c_str());
	memcpy(command->str, str.c_str(), cmd->length);
	return _write(cmd, Type::Str, option);
}

int SerialBase_::_write(const uint8_t *buf, int length, CommandMicroappLogOption option) {
	uint8_t *payload = getOutgoingMessagePayload();
	//io_buffer_t *buffer = getOutgoingMessageBuffer();
	microapp_log_array_cmd_t* command = reinterpret_cast<microapp_log_array_cmd_t*>(payload);
	microapp_log_cmd_t *cmd = reinterpret_cast<microapp_log_cmd_t*>(command);
	cmd->length = length;
	memcpy(command->arr, buf, cmd->length);
	return _write(cmd, Type::Arr, option);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
int SerialBase_::_write(microapp_log_cmd_t *cmd, Type type, CommandMicroappLogOption option) {
	cmd->port = _port;
	switch(_port) {
	case MICROAPP_SERIAL_SERVICE_DATA_PORT_NUMBER:
		cmd->header.cmd = CS_MICROAPP_COMMAND_SERVICE_DATA;
		break;
	case MICROAPP_SERIAL_DEFAULT_PORT_NUMBER:
	default:
		cmd->header.cmd = CS_MICROAPP_COMMAND_LOG;
		break;
	}
	cmd->type = type;
	cmd->option = option;
	sendMessage();
	return cmd->length;
}

int SerialServiceData_::write(microapp_service_data_t *data) {
	uint8_t *arr = reinterpret_cast<uint8_t*>(data);
	return SerialBase_::write(arr, (int)sizeof(microapp_service_data_t));
}


