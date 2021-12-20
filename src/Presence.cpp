#include <Presence.h>

static const uint8_t MAX_ROOMS 64

uint64_t Presence::getPresence(uint8_t profileId) {

	global_msg.payload[0] = CS_MICROAPP_COMMAND_PRESENCE;

	microapp_presence_t* cmd_payload = (microapp_presence_t*)&global_msg.payload[1];
	cmd_payload->profileId           = profileId;
	cmd_payload->presenceBitmask     = 0;

	global_msg.length = 1 + sizeof(microapp_presence_t);

	sendMessage(&global_msg);

	return cmd_payload->presenceBitmask;
}
bool Presence::isPresent(uint8_t profileId, uint8_t roomId) {

	if (roomId > MAX_ROOMS - 1) {
		return false;
	}

	uint64_t presenceBitmask = getPresence(profileId);

	return presenceBitmask & (1 << roomId);
}
