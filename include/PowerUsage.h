#pragma once

#include <microapp.h>

class PowerUsage{
public:
	// Empty constructor
	PowerUsage(){};

	/**
	 * Request the filtered power usage in milliwatts from bluenet
	 *
	 * @return uint32_t Filtered power usage in milliwatts
	 */
	int32_t getPowerUsageMilliWatts();
};
