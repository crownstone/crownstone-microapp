#include <ipc/cs_IpcRamData.h>
#include <microapp.h>

// Define array with soft interrupts
interrupt_registration_t interruptRegistrations[MAX_INTERRUPT_REGISTRATIONS];

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
 * Accessible by both bluenet and the microapp
 */
static bluenet_io_buffer_t shared_io_buffer;

/*
 * A global object for ipc data as well.
 */
static bluenet2microapp_ipcdata_t ipc_data;

uint8_t* getOutgoingMessagePayload() {
	return shared_io_buffer.microapp2bluenet.payload;
}

uint8_t* getIncomingMessagePayload() {
	return shared_io_buffer.bluenet2microapp.payload;
}

/*
 * Struct that stores copies of the shared io buffer
 */
struct stack_entry_t {
	// uint8_t interruptBuffer[MICROAPP_SDK_MAX_PAYLOAD];
	// uint8_t requestBuffer[MICROAPP_SDK_MAX_PAYLOAD];
	bluenet_io_buffer_t ioBuffer;
	uint8_t filled = false;
};

/*
 * Defines how 'deep' nested interrupts can go.
 * For each level, an interrupt buffer and a request buffer are needed to store the state of the level above.
 */
static const uint8_t MAX_INTERRUPT_DEPTH = 3;

/*
 * Stack of io buffer copies. For each interrupt layer, the stack grows
 */
static stack_entry_t stack[MAX_INTERRUPT_DEPTH];

static bool stack_initialized = false;

/*
 * Function checkRamData is used in sendMessage.
 */
int checkRamData(bool checkOnce) {
	int result = -1;

	if (!stack_initialized) {
		for (int8_t i = 0; i < MAX_INTERRUPT_DEPTH; ++i) {
			stack[i].filled = false;
		}
		stack_initialized = true;
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
int8_t emptySlotsInStack() {
	int8_t totalEmpty = 0;
	for (int8_t i = 0; i < MAX_INTERRUPT_REGISTRATIONS; ++i) {
		if (!stack[i].filled) {
			totalEmpty++;
		}
	}
	return totalEmpty;
}

int8_t getNextEmptyStackSlot() {
	for (uint8_t i = 0; i < MAX_INTERRUPT_REGISTRATIONS; ++i) {
		if (!stack[i].filled) {
			return i;
		}
	}
	return -1;
}

/*
 * Handle incoming interrupts from bluenet
 */
int handleBluenetInterrupt() {
	uint8_t* incomingPayload = getIncomingMessagePayload();
	microapp_sdk_header_t* incomingHeader = reinterpret_cast<microapp_sdk_header_t*>(incomingPayload);
	// First check if this is not just a regular call
	if (incomingHeader->ack != CS_ACK_REQUEST) {
		// No request, so this is not an interrupt
		return;
	}
	// Check if we have the capacity to handle another interrupt
	int8_t emptySlots = emptySlotsInStack();
	if (emptySlots == 0) {
		// Max depth has been reached, drop the interrupt and return
		incomingHeader->ack = CS_ACK_ERR_BUSY;
		// Yield to bluenet, without writing in the outgoing buffer.
		// Bluenet will check the written ack field
		sendMessage();
		return;
	}
	// Get the index of the first empty stack slot
	int8_t stackIndex = getNextEmptyStackSlot();
	if (stackIndex < 0) {
		// Apparently there was no space. Should not happen since we just checked
		// In any case, let's just drop and return similarly to above
		incomingHeader->ack = CS_ACK_ERR_BUSY;
		sendMessage();
		return;
	}
	// Copy the shared buffers to the top of the stack
	stack_entry_t* newStackEntry = &stack[stackIndex];
	uint8_t* outgoingPayload = getOutgoingMessagePayload();
	// Copying the outgoing buffer is needed so that the outgoing buffer can be freely used
	// for microapp requests during the interrupt handling
	memcpy(newStackEntry->ioBuffer.microapp2bluenet, outgoingPayload, MICROAPP_SDK_MAX_PAYLOAD);
	// Copying the incoming buffer is needed so that the interrupt payload is preserved
	// if bluenet generates another interrupt before finishing handling this one
	memcpy(newStackEntry->ioBuffer.bluenet2microapp, incomingPayload, MICROAPP_SDK_MAX_PAYLOAD);
	newStackEntry->filled = true;

	// Mark the incoming ack as 'in progress' so bluenet will keep calling
	incomingHeader->ack = CS_ACK_IN_PROGRESS;

	// Now the interrupt will actually be handled
	// Call the interrupt handler and pass a pointer to the copy of the interrupt buffer
	// Note that new sendMessage calls may occur in the interrupt handler
	microapp_sdk_header_t* stackEntryHeader = reinterpret_cast<microapp_sdk_header_t*>(newStackEntry->ioBuffer.bluenet2microapp);
	int result = handleInterrupt(stackEntryHeader);

	// When done with the interrupt handling, we can pop the buffers from the stack again
	// Though really we only need the outgoing buffer, since we just finished dealing with the incoming buffer
	memcpy(outgoingPayload, newStackEntry->ioBuffer.microapp2bluenet, MICROAPP_SDK_MAX_PAYLOAD);
	newStackEntry->filled = false;

	// End with a sendMessage call which yields back to bluenet
	// Bluenet will see the
	incomingHeader->ack = CS_ACK_SUCCESS;
	sendMessage();

	// call handleSoftInterrupt and mark the localQueue entry as empty again
	// Q: Under what circumstances will the queue grow beyond 1 entry?
	// A: If bluenet comes with a second request before the first softInterrupt is handled
	// That only seems possible if handleSoftInterrupt yields back to bluenet
	// which tbh I don't see happening easily, maybe with a delay call in a user softInterrupt handler?
	// It also happens when bluenet forces a yield due to consecutive calls
	queue_t* localCopy = &localQueue[queueIndex];
	memcpy(localCopy->buffer, request, MAX_PAYLOAD);
	microapp_cmd_t* msg = reinterpret_cast<microapp_cmd_t*>(localCopy->buffer);
	result              = handleSoftInterrupt(msg);
	localCopy->filled   = false;

	// End with a call to sendMessage(). It will not enter this function again because
	// the microapp itself changed request->ack to something else than CS_ACK_BLUENET_MICROAPP_REQUEST.

	uint8_t *payload = getOutgoingMessagePayload();
	microapp_cmd_t* outCmd = (microapp_cmd_t*)(payload);
	if (result != ERR_MICROAPP_SUCCESS) {
		outCmd->cmd = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_ERROR;
	}
	else {
		outCmd->cmd = CS_MICROAPP_COMMAND_SOFT_INTERRUPT_END;
	}
	outCmd->id = request->header.id;
	sendMessage();
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

	// Here the microapp resumes execution, check for incoming interrupts
	handleBluenetRequest();

	return result;
}

int registerSoftInterrupt(interrupt_registration_t* interrupt) {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (!interruptRegistrations[i].registered) {
			interruptRegistrations[i].registered        = true;
			interruptRegistrations[i].softInterruptFunc = interrupt->softInterruptFunc;
			interruptRegistrations[i].id                = interrupt->id;
			interruptRegistrations[i].arg               = interrupt->arg;
			interruptRegistrations[i].type              = interrupt->type;
			return ERR_MICROAPP_SUCCESS;
		}
	}
	return ERR_MICROAPP_NO_SPACE;
}

int removeRegisteredSoftInterrupt(uint8_t type, uint8_t id) {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (!interruptRegistrations[i].registered) {
			continue;
		}
		if (interruptRegistrations[i].type == type && interruptRegistrations[i].id == id) {
			interruptRegistrations[i].registered = false;
			return ERR_MICROAPP_SUCCESS;
		}
	}
	return ERR_MICROAPP_SOFT_INTERRUPT_NOT_REGISTERED;
}

int countRegisteredSoftInterrupts() {
	int result = 0;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (interruptRegistrations[i].registered) {
			result++;
		}
	}
	return result;
}

int countRegisteredSoftInterrupts(uint8_t type) {
	int result = 0;
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (interruptRegistrations[i].registered && interruptRegistrations[i].type == type) {
			result++;
		}
	}
	return result;
}

int handleSoftInterruptInternal(uint8_t type, uint8_t id, uint8_t* msg) {
	for (int i = 0; i < MAX_SOFT_INTERRUPTS; ++i) {
		if (!interruptRegistrations[i].registered) {
			continue;
		}
		if ( interruptRegistrations[i].type != type) {
			continue;
		}
		if (interruptRegistrations[i].id == id) {
			if (interruptRegistrations[i].softInterruptFunc) {
				return interruptRegistrations[i].softInterruptFunc(interruptRegistrations[i].arg, msg);
			}
			// softInterruptFunc does not exist
			return ERR_MICROAPP_SOFT_INTERRUPT_NOT_REGISTERED;
		}
	}
	 // no soft interrupt of this type with this id registered
	return ERR_MICROAPP_SOFT_INTERRUPT_NOT_REGISTERED;
}

int handleSoftInterrupt(microapp_cmd_t* msg) {
	int result = ERR_MICROAPP_SUCCESS;
	switch (msg->interruptCmd) {
		case CS_MICROAPP_COMMAND_BLE_DEVICE: {
			result = handleSoftInterruptInternal(SOFT_INTERRUPT_TYPE_BLE, msg->id, (uint8_t*)msg);
			break;
		}
		case CS_MICROAPP_COMMAND_PIN: {
			result = handleSoftInterruptInternal(SOFT_INTERRUPT_TYPE_PIN, msg->id, (uint8_t*)msg);
			break;
		}
		case CS_MICROAPP_COMMAND_MESH: {
			result = handleSoftInterruptInternal(SOFT_INTERRUPT_TYPE_MESH, msg->id, (uint8_t*)msg);
			break;
		}
		default: {
			// interruptCmd not known
			return ERR_MICROAPP_UNKNOWN_PROTOCOL;
		}
	}
	return result;
}
