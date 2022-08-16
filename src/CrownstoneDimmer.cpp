#include <CrownstoneDimmer.h>
#include <Serial.h>

/*
 * Sends a message to bluenet with a dimming request.
 * Complies with the control command protocol
 */
void CrownstoneDimmer::setIntensity(uint8_t intensity) {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(payload);
	switchRequest->header.ack            = CS_ACK_REQUEST;
	switchRequest->header.sdkType        = CS_MICROAPP_SDK_TYPE_SWITCH;
	if (intensity > 100) {
		intensity = 100;
	}
	switchRequest->value = intensity;
	sendMessage();
}