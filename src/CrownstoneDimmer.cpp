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
	microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(payload);
	switchRequest->header.ack            = CS_ACK_REQUEST;
	switchRequest->header.sdkType        = CS_MICROAPP_SDK_TYPE_SWITCH;
	if (intensity > 100) {
		intensity = 100;
	}
	switchRequest->value = intensity;
	sendMessage();
}