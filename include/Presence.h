#pragma once

#include <microapp.h>

class Presence {
public:
	/**
	 * Get a bitmask containing rooms for a single profile with ID 
	 *
	 * @param[out] profileId  ID of the user profile we want to check.
	 * @return                64 bit bitmask for a single profile. If the Nth bit is set, the profile is present in the
	 * Nth room. If bit 0 is set, the profile is present in sphere.
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
