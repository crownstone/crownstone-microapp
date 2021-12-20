#pragma once

#include <microapp.h>

class Presence {
public:
	// Check if Profile is in sphere
	uint64_t getPresence(uint8_t profileId);

	// Check if profile is in a Room
	bool isPresent(uint8_t profileId, uint8_t roomId);
};
