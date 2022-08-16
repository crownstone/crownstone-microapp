#pragma once

#include <Arduino.h>

/*
 * Class for controlling the crownstone dimmer
 * Crownstones need to be dimming-compatible
 */
class CrownstoneDimmer {
public:
	// Default constructor
	CrownstoneDimmer(){};

	// Set dimming intensity.
	// The intensity should be a percentage between 0 and 100.
	void setIntensity(uint8_t intensity);

};