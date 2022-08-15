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
	bluenet_io_buffer_t ioBuffer;
	bool filled;
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
microapp_result_t checkRamData(bool checkOnce) {
	if (!stack_initialized) {
		for (int8_t i = 0; i < MAX_INTERRUPT_DEPTH; ++i) {
			stack[i].filled = false;
		}
		stack_initialized = true;
	}

	if (checkOnce) {
		// If valid is set, we assume cached values are fine, otherwise load them.
		if (ipc_data.valid) {
			return CS_ACK_SUCCESS;
		}
	}

	uint8_t rd_size = 0;
	uint8_t ret_code =
			getRamData(IPC_INDEX_CROWNSTONE_APP, (uint8_t*)&ipc_data, sizeof(bluenet2microapp_ipcdata_t), &rd_size);

	if (ret_code != 0) {
		return CS_ACK_ERROR;
	}

	if (ipc_data.length != sizeof(bluenet2microapp_ipcdata_t)) {
		return CS_ACK_ERROR;
	}

	if (ipc_data.protocol != 1) {
		return CS_ACK_ERROR;
	}

	if (!ipc_data.microappCallback) {
		return CS_ACK_ERROR;
	}

	ipc_data.valid = true;

	if (checkOnce) {
		// Write the buffer only once
		microappCallbackFunc callbackFunctionIntoBluenet = ipc_data.microappCallback;
		microapp_result_t result = callbackFunctionIntoBluenet(CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER, &shared_io_buffer);
	}
	return CS_ACK_SUCCESS;
}

/*
 * Returns the number of empty slots for bluenet.
 */
uint8_t emptySlotsInStack() {
	uint8_t totalEmpty = 0;
	for (uint8_t i = 0; i < MAX_INTERRUPT_DEPTH; ++i) {
		if (!stack[i].filled) {
			totalEmpty++;
		}
	}
	return totalEmpty;
}

int8_t getNextEmptyStackSlot() {
	for (uint8_t i = 0; i < MAX_INTERRUPT_DEPTH; ++i) {
		if (!stack[i].filled) {
			return i;
		}
	}
	return -1;
}

/*
 * Handle incoming interrupts from bluenet
 */
void handleBluenetInterrupt() {
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
	memcpy(newStackEntry->ioBuffer.microapp2bluenet.payload, outgoingPayload, MICROAPP_SDK_MAX_PAYLOAD);
	// Copying the incoming buffer is needed so that the interrupt payload is preserved
	// if bluenet generates another interrupt before finishing handling this one
	memcpy(newStackEntry->ioBuffer.bluenet2microapp.payload, incomingPayload, MICROAPP_SDK_MAX_PAYLOAD);
	newStackEntry->filled = true;

	// Mark the incoming ack as 'in progress' so bluenet will keep calling
	incomingHeader->ack = CS_ACK_IN_PROGRESS;

	// Now the interrupt will actually be handled
	// Call the interrupt handler and pass a pointer to the copy of the interrupt buffer
	// Note that new sendMessage calls may occur in the interrupt handler
	microapp_sdk_header_t* stackEntryHeader = reinterpret_cast<microapp_sdk_header_t*>(newStackEntry->ioBuffer.bluenet2microapp.payload);
	int result = handleInterrupt(stackEntryHeader);

	// When done with the interrupt handling, we can pop the buffers from the stack again
	// Though really we only need the outgoing buffer, since we just finished dealing with the incoming buffer
	memcpy(outgoingPayload, newStackEntry->ioBuffer.microapp2bluenet.payload, MICROAPP_SDK_MAX_PAYLOAD);
	newStackEntry->filled = false;

	// End with a sendMessage call which yields back to bluenet
	// Bluenet will see the acknowledge and not call again
	incomingHeader->ack = result;
	sendMessage();
	return;
}

/*
 * Send the actual message to bluenet
 *
 * If there are no interrupts it will just return and at some later time be called again.
 */
microapp_result_t sendMessage() {
	bool checkOnce = true;
	microapp_result_t result = checkRamData(checkOnce);
	if (result != CS_ACK_SUCCESS) {
		return result;
	}

	// The callback will yield control to bluenet.
	microappCallbackFunc callbackFunctionIntoBluenet = ipc_data.microappCallback;
	uint8_t opcode = checkOnce ? CS_MICROAPP_CALLBACK_SIGNAL : CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER;
	result         = callbackFunctionIntoBluenet(opcode, &shared_io_buffer);

	// Here the microapp resumes execution, check for incoming interrupts
	handleBluenetInterrupt();

	return result;
}

microapp_result_t registerInterrupt(interrupt_registration_t* interrupt) {
	for (int i = 0; i < MAX_INTERRUPT_REGISTRATIONS; ++i) {
		if (!interruptRegistrations[i].registered) {
			interruptRegistrations[i].registered = true;
			interruptRegistrations[i].handler    = interrupt->handler;
			interruptRegistrations[i].major      = interrupt->major;
			interruptRegistrations[i].minor      = interrupt->minor;
			return CS_ACK_SUCCESS;
		}
	}
	return CS_ACK_ERR_NO_SPACE;
}

microapp_result_t removeInterruptRegistration(uint8_t major, uint8_t minor) {
	for (int i = 0; i < MAX_INTERRUPT_REGISTRATIONS; ++i) {
		if (!interruptRegistrations[i].registered) {
			continue;
		}
		if (interruptRegistrations[i].major == major &&
			interruptRegistrations[i].minor == minor) {
			interruptRegistrations[i].registered = false;
			return CS_ACK_SUCCESS;
		}
	}
	return CS_ACK_ERR_NOT_FOUND;
}

microapp_result_t callInterrupt(uint8_t major, uint8_t minor, microapp_sdk_header_t* interruptHeader) {
	for (int i = 0; i < MAX_INTERRUPT_REGISTRATIONS; ++i) {
		if (!interruptRegistrations[i].registered) {
			continue;
		}
		if (interruptRegistrations[i].major != major) {
			continue;
		}
		if (interruptRegistrations[i].minor == minor) {
			if (interruptRegistrations[i].handler) {
				return interruptRegistrations[i].handler(interruptHeader);
			}
			// Handler does not exist
			return CS_ACK_ERR_NOT_FOUND;
		}
	}
	 // No soft interrupt of this type with this id registered
	return CS_ACK_ERR_NOT_FOUND;
}

microapp_result_t handleInterrupt(microapp_sdk_header_t* interruptHeader) {
	// For all possible interrupt types, get the minor from the incoming message
	uint8_t minor;
	switch (interruptHeader->sdkType) {
		case CS_MICROAPP_SDK_TYPE_PIN: {
			microapp_sdk_pin_t* pinInterrupt = reinterpret_cast<microapp_sdk_pin_t*>(interruptHeader);
			minor = pinInterrupt->pin;
			break;
		}
		case CS_MICROAPP_SDK_TYPE_BLE: {
			microapp_sdk_ble_t* bleInterrupt = reinterpret_cast<microapp_sdk_ble_t*>(interruptHeader);
			minor = bleInterrupt->type;
			break;
		}
		case CS_MICROAPP_SDK_TYPE_MESH: {
			microapp_sdk_mesh_t* meshInterrupt = reinterpret_cast<microapp_sdk_mesh_t*>(interruptHeader);
			minor = meshInterrupt->type;
			break;
		}
		default: {
			return CS_ACK_ERR_UNDEFINED;
		}
	}
	// Call the interrupt
	return callInterrupt(interruptHeader->sdkType, minor, interruptHeader);
}
