#include <microapp.h>
#include <ipc/cs_IpcRamData.h>

// Define array with callbacks
callback_t callbacks[MAX_CALLBACKS];

// Important: Do not include <string.h> / <cstring>. This bloats the binary unnecessary.
// On Arduino there is the String class. Roll your own functions like strlen, see below.

// returns size MAX_STRING_SIZE for strings that are too long, note that this can still not fit in the payload
// the actually supported string length depends on the opcode
// the limit here is just to prevent looping forever
uint8_t strlen(const char *str) {
	for (size_t i = 0; i < MAX_STRING_SIZE; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return MAX_STRING_SIZE;
}

// compares two buffers of length num, ptr1 and ptr2
// returns 0 if ptr1 and ptr2 are equal
// returns -1 if for the first unmatching byte i we have ptr1[i] < ptr2[i]
// returns 1 if for the first unmatching byte i we have ptr1[i] > ptr2[i]
int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	char *p = (char*) ptr1;
	char *q = (char*) ptr2;
	if (ptr1 == ptr2) { // point to the same address
		return 0;
	}
	for (size_t i = 0; i < num; ++i) {
		if (*(p+i) < *(q+i)) {
			return -1;
		}
		else if (*(p+i) > *(q+i)) {
			return 1;
		}
	}
	return 0;
}

void* memcpy(void* dest, const void* src, size_t num) {
	uint8_t* p = (uint8_t*) src;
	uint8_t* q = (uint8_t*) dest;
	for (size_t i = 0; i < num; ++i) {
		*q = *p;
		p++;
		q++;
	}
	return dest;
}

/*
 * A global object to send a message.
 */
microapp_message_t global_msg;

/*
 * A global object for ipc data as well.
 */
bluenet2microapp_ipcdata_t ipc_data;

/*
 * Send the actual message.
 */
int sendMessage(microapp_message_t *msg) {
	int result = -1;

	// Check length.
	if (msg->length > MAX_PAYLOAD) {
		return result;
	}

	// If valid is set to 1, we assume cached values are fine, otherwise load them.
	if(!ipc_data.valid) {
		uint8_t rd_size = 0;
		uint8_t ret_code = 
			getRamData(IPC_INDEX_CROWNSTONE_APP, (uint8_t*)&ipc_data, sizeof(bluenet2microapp_ipcdata_t), &rd_size);

		if (ret_code != 0) {
			return result;
		}

		if (ipc_data.length != sizeof(bluenet2microapp_ipcdata_t)) {
			return result;
		}

		if(ipc_data.protocol != 1) {
			return result;
		}

		if (!ipc_data.microapp_callback) {
			return result;
		}
		ipc_data.valid = true;
	}

	// The callback will yield control to bluenet.
	int (*callback_func)(uint8_t*,uint16_t) = (int (*)(uint8_t*,uint16_t)) ipc_data.microapp_callback;
	result = callback_func(msg->payload, msg->length);

	// Here the microapp resumes execution
	microapp_cmd_t *buf = reinterpret_cast<microapp_cmd_t*>(msg->payload);

	// A command is indicated as being processed by CS_MICROAPP_COMMAND_NONE.
	// Somehow this is often NOT set to COMMAND_NONE!
	if(buf->cmd != CS_MICROAPP_COMMAND_NONE) {
		// This is probably a callback.
		result = handleCallbacks(buf);

		// Immediately return again
		buf->cmd = CS_MICROAPP_COMMAND_CALLBACK_DONE;
		buf->id = result + 100;

		int (*callback_func)(uint8_t*,uint16_t) = (int (*)(uint8_t*,uint16_t)) ipc_data.microapp_callback;
		result = callback_func(msg->payload, msg->length);
	}

	if(buf->ack) {
		buf->ack = false;
		
	}

	return result;
}

void registerCallback(callback_t *cb) {
	for (int i = 0; i < MAX_CALLBACKS; ++i) {
		if (callbacks[i].empty) {
			callbacks[i].empty = false;
			callbacks[i].callback = cb->callback;
			callbacks[i].id = cb->id;
			callbacks[i].arg = cb->arg;
		}
	}
}

int handleCallbacks(microapp_cmd_t *msg) {
	int result = -4;
	switch(msg->cmd) {
	case CS_MICROAPP_COMMAND_BLE_DEVICE: {
		// get somehow callback id
		for (int i = 0; i < MAX_CALLBACKS; ++i) {
			if (callbacks[i].empty || callbacks[i].type != CALLBACK_TYPE_BLE) {
				continue;
			}
			if (callbacks[i].id == msg->id) {
				if (callbacks[i].callback) {
					callbacks[i].callback(callbacks[i].arg);
					result = msg->id;
					break;
				}
			}
		}
		break;
	}
	case CS_MICROAPP_COMMAND_PIN: {
		result = -5;
		microapp_pin_cmd_t* cmd = reinterpret_cast<microapp_pin_cmd_t*>(msg);
		if (cmd->value != CS_MICROAPP_COMMAND_VALUE_CHANGE) {
			break;
		}
		for (int i = 0; i < MAX_CALLBACKS; ++i) {
			if (callbacks[i].empty || callbacks[i].type != CALLBACK_TYPE_PIN) {
				continue;
			}
	//		if (callbacks[i].id == cmd->pin) {
				if (callbacks[i].callback) {
					callbacks[i].callback(callbacks[i].arg);
					result = callbacks[i].id;
					break;
				}
	//		}
		}
		break;
	}
	}
	result = msg->cmd;
	return result;
}
