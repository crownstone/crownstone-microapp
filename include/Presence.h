#pragma once

#include <microapp.h>

const uint8_t MAX_ROOMS = 64;

class Presence {
private:
	/**
	 * Request the presence bitmask of a profile from bluenet
	 *
	 * @param profileId   The id of the profile for which to request presence
	 * @return bitmask    A 64-bit bitmask for a single profile. If the Nth bit is set,
	 *                    the profile is present in the Nth room. Bit 0 represents the sphere
	 */
	uint64_t getPresence(uint8_t profileId);

public:
	// Empty constructor
	Presence(){};

	/**
	 * Check if a profile is present in a room or in the sphere
	 *
	 * @param profileId Id of the profile. Use 0 for all users
	 * @param roomId    Id of the room. When 0, check for the whole sphere
	 * @return          True if profile present in the room, false if not
	 */
	bool isPresent(uint8_t profileId, uint8_t roomId);
};
