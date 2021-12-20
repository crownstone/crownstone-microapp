#pragma once

#include <microapp.h>

class Presence {
public:
	/**
	 * Check if profile is present in the sphere.
	 *
	 * @param[out] profileId  ID of the user profile we want to check.
	 * @return                64 byte Intiger with a bit field of available rooms, in which a profile is present or not.
	 */
	uint64_t getPresence(uint8_t profileId);

	/**
	 * Check if profile is present in a room.
	 *
	 * @param[out] profileId  ID of the user profile we want to check.
	 * @param[out] roomId	  ID of the room we want to check.
	 * @return                True if user profile present in room, false if not.
	 */
	bool isPresent(uint8_t profileId, uint8_t roomId);
};
