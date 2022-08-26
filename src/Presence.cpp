#include <Presence.h>

uint64_t Presence::getPresence(uint8_t profileId) {
	uint8_t* out                             = getOutgoingMessagePayload();
	microapp_sdk_presence_t* presenceRequest = reinterpret_cast<microapp_sdk_presence_t*>(out);
	presenceRequest->header.messageType      = CS_MICROAPP_SDK_TYPE_PRESENCE;
	presenceRequest->header.ack              = CS_MICROAPP_SDK_ACK_REQUEST;
	presenceRequest->profileId               = profileId;
	presenceRequest->presenceBitmask         = 0;
	sendMessage();
	if (presenceRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return 0;
	}
	return presenceRequest->presenceBitmask;
}

bool Presence::isPresent(uint8_t profileId, uint8_t roomId) {
	if (roomId >= MAX_ROOMS) {
		return false;
	}
	uint64_t presenceBitmask = getPresence(profileId);
	return presenceBitmask & (1 << roomId);
}