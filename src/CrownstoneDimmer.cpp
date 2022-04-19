#include <CrownstoneDimmer.h>
#include <Serial.h>

void CrownstoneDimmer::init() {
	_initialized = true;
}

void CrownstoneDimmer::setIntensity(uint8_t intensity) {
	if (!_initialized) {
		Serial.println("Dimmer not initialized");
		return;
	}
	uint8_t *payload = getOutgoingMessagePayload();
	microapp_dimmer_switch_cmd_t* dimmer_cmd = reinterpret_cast<microapp_dimmer_switch_cmd_t*>(payload);
	dimmer_cmd->header.cmd = CS_MICROAPP_COMMAND_SWITCH_DIMMER;
	dimmer_cmd->opcode = CS_MICROAPP_COMMAND_DIMMER;
	dimmer_cmd->value = intensity;

	sendMessage();
}