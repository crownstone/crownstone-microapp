#pragma once

#include <microapp.h>

class Presence{
public:
	uint64_t getPresence(uint8_t profileId);
	bool isPresent(uint8_t profileId, uint8_t roomId);
};
