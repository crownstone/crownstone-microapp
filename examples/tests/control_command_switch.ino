#include <Arduino.h>

boolean state = LOW;

void setup() {
}

void loop() {
	state = !state;

	uint8_t* payload = getOutgoingMessagePayload();
	microapp_sdk_control_command_t* controlCommand = reinterpret_cast<microapp_sdk_control_command_t*>(payload);
	controlCommand->header.messageType = CS_MICROAPP_SDK_TYPE_CONTROL_COMMAND;
	controlCommand->header.ack = CS_MICROAPP_SDK_ACK_REQUEST;
	controlCommand->protocol = 5;
	controlCommand->type = 20; // switch
	controlCommand->size = 1;
	if (state) {
		*controlCommand->payload = 100;
	}
	else {
		*controlCommand->payload = 0;
	}
	sendMessage();
}
