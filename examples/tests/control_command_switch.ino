#include <Arduino.h>

/**
 * This app showcases how the custom control command sdk protocol can be used
 * It toggles the relay just like the relay.ino, but via a control command
 */

boolean state = LOW;

void setup() {
	Serial.println("Control command switch example");
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
		controlCommand->payload[0] = 100;
	}
	else {
		controlCommand->payload[0] = 0;
	}
	sendMessage();
}
