#include <ipc/cs_IpcRamData.h>
#include <microapp.h>

// Define array with soft interrupts
soft_interrupt_t softInterrupt[MAX_SOFT_INTERRUPTS];

// Important: Do not include <string.h> / <cstring>. This bloats up the binary unnecessary.
// On Arduino there is the String class. Roll your own functions like strlen, see below.

// returns size MAX_STRING_SIZE for strings that are too long, note that this can still not fit in the payload
// the actually supported string length depends on the opcode
// the limit here is just to prevent looping forever
uint8_t strlen(const char* str) {
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
int memcmp(const void* ptr1, const void* ptr2, size_t num) {
	char* p = (char*)ptr1;
	char* q = (char*)ptr2;
	if (ptr1 == ptr2) {  // point to the same address
		return 0;
	}
	for (size_t i = 0; i < num; ++i) {
		if (*(p + i) < *(q + i)) {
			return -1;
		}
		else if (*(p + i) > *(q + i)) {
			return 1;
		}
	}
	return 0;
}

void* memcpy(void* dest, const void* src, size_t num) {
	uint8_t* p = (uint8_t*)src;
	uint8_t* q = (uint8_t*)dest;
	for (size_t i = 0; i < num; ++i) {
		*q = *p;
		p++;
		q++;
	}
	return dest;
}

/*
 * A global object for messages in and out.
 */
static bluenet_io_buffer_t io_buffer;

/*
 * A global object for ipc data as well.
 */
static bluenet2microapp_ipcdata_t ipc_data;

uint8_t* getOutgoingMessagePayload() {
	return io_buffer.microapp2bluenet.payload;
}

uint8_t* getIncomingMessagePayload() {
	return io_buffer.bluenet2microapp.payload;
}

/*
 * Have a queue with a couple of buffers. Incoming messages can be placed in the first empty spot and afterwards
 * being handled.
 */
struct queue_t {
	uint8_t buffer[MAX_PAYLOAD];
	uint8_t filled;
};

static const uint8_t MAX_QUEUE = 3;

/*
 * A set of local buffers for incoming messages.
 */
static queue_t localQueue[MAX_QUEUE];

static int buffer_initialized = false;

/*
 * Function checkRamData is used in sendMessage.
 */
int checkRamData(bool checkOnce) {
	int result = -1;

	if (!buffer_initialized) {
		for (int8_t i = 0; i < MAX_QUEUE; ++i) {
			localQueue[i].filled = false;
		}
		buffer_initialized = true;
	}

	if (checkOnce) {
		// If valid is set, we assume cached values are fine, otherwise load them.
		if (ipc_data.valid) {
			return 0;
		}
	}

	uint8_t rd_size = 0;
	uint8_t ret_code =
			getRamData(IPC_INDEX_CROWNSTONE_APP, (uint8_t*)&ipc_data, sizeof(bluenet2microapp_ipcdata_t), &rd_size);

	if (ret_code != 0) {
		return result;
	}

	if (ipc_data.length != sizeof(bluenet2microapp_ipcdata_t)) {
		return result;
	}

	if (ipc_data.protocol != 1) {
		return result;
	}

	if (!ipc_data.microappCallback) {
		return result;
	}

	ipc_data.valid = true;

	if (checkOnce) {
		// Write the buffer only once
		microappCallbackFunc callbackFunctionIntoBluenet = ipc_data.microappCallback;
		result = callbackFunctionIntoBluenet(CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER, &io_buffer);
	}
	return 0;
}

/*
 * Returns the number of empty slots for bluenet. Access and counting of empty slots can be improved.
 */
int8_t emptySlotsInQueue() {
	int8_t totalEmpty = 0;
	for (int8_t i = 0; i < MAX_QUEUE; ++i) {
		if (!localQueue[i].filled) {
			totalEmpty++;
		}
	}
	return totalEmpty;
}

/*
 * Gets a new slide.
 */
int8_t getNewItemInQueue() {
	int8_t queueIndex = -1;
	for (int8_t i = 0; i < MAX_QUEUE; ++i) {
		if (!localQueue[i].filled) {
			queueIndex           = i;
			localQueue[i].filled = true;
			break;
		}
	}
	return queueIndex;
}

/*
 * Handle incoming requests from bluenet (probably soft interrupts).
 */
int handleBluenetRequest(microapp_cmd_t* cmd) {
	int result = 1;

	// There is no actual request (no problem, just return)
	if (cmd->ack != CS_ACK_BLUENET_MICROAPP_REQUEST) {
		return 0;
	}

	microapp_soft_interrupt_cmd_t* request = reinterpret_cast<microapp_soft_interrupt_cmd_t*>(cmd);

	// Check if there are empty slots
	int8_t emptySlots = emptySlotsInQueue();

	int8_t queueIndex = -1;
	if (emptySlots > 0) {
		queueIndex                   = getNewItemInQueue();
		request->emptyInterruptSlots = emptySlots - 1;
	}
	else {
		request->emptyInterruptSlots = emptySlots;
	}

	// overwrite in both scenarios the REQUEST
	if (queueIndex < 0) {
		request->header.ack = CS_ACK_BLUENET_MICROAPP_REQ_BUSY;
	}
	else {
		request->header.ack = CS_ACK_BLUENET_MICROAPP_REQ_ACK;
	}

	// First indicate we have received the callback
	uint8_t* outputPayloadRaw     = getOutgoingMessagePayload();
	microapp_cmd_t* outputPayload = reinterpret_cast<microapp_cmd_t*>(outputPayloadRaw);
	if (queueIndex < 0) {
		outputPayload->cmd = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_ERROR;
	}
	else {
		outputPayload->cmd = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_RECEIVED;
	}

	microappCallbackFunc callbackFunctionIntoBluenet = ipc_data.microappCallback;
	result = callbackFunctionIntoBluenet(CS_MICROAPP_CALLBACK_SIGNAL, &io_buffer);

	if (queueIndex < 0) {
		return -1;
	}

	// Continue execution, this will end up with a call to sendMessage() but it will not enter this function because
	// the microapp itself changed request->ack to something else than CS_ACK_BLUENET_MICROAPP_REQUEST.

	queue_t* localCopy = &localQueue[queueIndex];
	memcpy(localCopy->buffer, request, MAX_PAYLOAD);
	microapp_cmd_t* msg = reinterpret_cast<microapp_cmd_t*>(localCopy->buffer);
	result              = handleSoftInterrupt(msg);
	localCopy->filled   = false;
	return result;
}

/*
 * Send the actual message.
 *
 * This method will also obtain information for soft interrupts and propagate those if necessary. If there are no
 * soft interrupts it will just return and at some later time be called again.
 */
int sendMessage() {
	bool checkOnce = true;
	int result     = checkRamData(checkOnce);
	if (result < 0) {
		return result;
	}

	// The callback will yield control to bluenet.
	microappCallbackFunc callbackFunctionIntoBluenet = ipc_data.microappCallback;
	uint8_t opcode = checkOnce ? CS_MICROAPP_CALLBACK_SIGNAL : CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER;
	result         = callbackFunctionIntoBluenet(opcode, &io_buffer);

	// Here the microapp resumes execution, check for incoming messages
	microapp_cmd_t* inputBuffer = reinterpret_cast<microapp_cmd_t*>(io_buffer.bluenet2microapp.payload);

	result = handleBluenetRequest(inputBuffer);

	return result;
}

void registerSoftInterrupt(soft_interrupt_t* interrupt) {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (!softInterrupt[i].registered) {
			softInterrupt[i].registered        = true;
			softInterrupt[i].softInterruptFunc = interrupt->softInterruptFunc;
			softInterrupt[i].id                = interrupt->id;
			softInterrupt[i].arg               = interrupt->arg;
			softInterrupt[i].type              = interrupt->type;
			break;
		}
	}
}

int countRegisteredSoftInterrupts() {
	int result = 0;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (softInterrupt[i].registered) {
			result++;
		}
	}
	return result;
}

int countRegisteredSoftInterrupts(uint8_t type) {
	int result = 0;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (softInterrupt[i].registered && softInterrupt[i].type == type) {
			result++;
		}
	}
	return result;
}

int handleSoftInterruptInternal(uint8_t type, uint8_t id, uint8_t* msg) {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (!softInterrupt[i].registered || softInterrupt[i].type != type) {
			continue;
		}
		if (softInterrupt[i].id == id) {
			if (softInterrupt[i].softInterruptFunc) {
				softInterrupt[i].softInterruptFunc(softInterrupt[i].arg, msg);
				return id;
			}
			return -2;
		}
	}
	return -1;
}

int handleSoftInterrupt(microapp_cmd_t* msg) {
	int result = 0;
	switch (msg->interruptCmd) {
		case CS_MICROAPP_COMMAND_BLE_DEVICE: {
			result = handleSoftInterruptInternal(SOFT_INTERRUPT_TYPE_BLE, msg->id, (uint8_t*)msg);
			break;
		}
		case CS_MICROAPP_COMMAND_PIN: {
			microapp_pin_cmd_t* cmd = reinterpret_cast<microapp_pin_cmd_t*>(msg);
			if (cmd->value != CS_MICROAPP_COMMAND_VALUE_CHANGE) {
				break;
			}
			result = handleSoftInterruptInternal(SOFT_INTERRUPT_TYPE_PIN, cmd->pin, (uint8_t*)msg);
			break;
		}
	}
	return result;
}
