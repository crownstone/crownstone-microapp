#include <Presence.h>

PresenceClass::PresenceClass() {}

uint64_t PresenceClass::getPresence(uint8_t profileId) {
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

bool PresenceClass::isPresent(uint8_t profileId, uint8_t roomId) {
	if (roomId >= MAX_ROOMS) {
		return false;
	}
	uint64_t presenceBitmask = getPresence(profileId);
	return presenceBitmask & (1 << roomId);
}
