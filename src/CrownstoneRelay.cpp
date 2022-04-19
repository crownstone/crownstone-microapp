#include <CrownstoneRelay.h>
#include <Serial.h>

void CrownstoneRelay::init() {
	_initialized = true;
}

void CrownstoneRelay::switchOff() {
	setSwitch(CS_MICROAPP_COMMAND_SWITCH_OFF);
}

void CrownstoneRelay::switchOn() {
	setSwitch(CS_MICROAPP_COMMAND_SWITCH_ON);
}

void CrownstoneRelay::switchToggle() {
	setSwitch(CS_MICROAPP_COMMAND_SWITCH_TOGGLE);
}

void CrownstoneRelay::setSwitch(CommandMicroappSwitchValue val) {
	if (!_initialized) {
		Serial.println("Relay not initialized");
		return;
	}
	uint8_t *payload = getOutgoingMessagePayload();
	microapp_dimmer_switch_cmd_t* switch_cmd = reinterpret_cast<microapp_dimmer_switch_cmd_t*>(payload);
	switch_cmd->header.cmd = CS_MICROAPP_COMMAND_SWITCH_DIMMER;
	switch_cmd->opcode = CS_MICROAPP_COMMAND_SWITCH;
	switch_cmd->value = val;

	sendMessage();
}