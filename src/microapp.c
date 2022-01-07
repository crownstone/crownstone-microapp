#include <microapp.h>
#include <ipc/cs_IpcRamData.h>

// Important: Do not include <string.h> / <cstring>. This bloats the binary unnecessary.
// On Arduino there is the String class. Roll your own functions like strlen, see below.

// returns size MAX_PAYLOAD for strings that are too long, note that this can still not fit in the payload
// the actually supported string length depends on the opcode
// the limit here is just to prevent looping forever
uint8_t strlen(const char *str) {
	for (uint8_t i = 0; i < MAX_PAYLOAD; ++i) {
		if (str[i] == 0) {
			return i;
		}
	}
	return MAX_PAYLOAD;
}

// compares two buffers of length len, bufA and bufB
// returns true if bufA and bufB are equal
bool memcmp(const void *bufA, const void *bufB, uint8_t len)
{
	char *p = (char*) bufA;
	char *q = (char*) bufB;
	for (uint8_t i = 0; i<len; i++)
	{
		if (*(p+i) != *(q+i) )
		{
			return false;
		}
	}
	return true;
}

/*
 * A global object to send a message.
 */
message_t global_msg;

/*
 * A global object for ipc data as well.
 */
bluenet2microapp_ipcdata_t ipc_data;

/*
 * Send the actual message.
 */
int sendMessage(message_t *msg) {
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

	// The actual callback
	int (*callback_func)(uint8_t*,uint16_t) = (int (*)(uint8_t*,uint16_t)) ipc_data.microapp_callback;
	result = callback_func(msg->payload, msg->length);
	return result;
}
