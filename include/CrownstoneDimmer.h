#pragma once

#include <Arduino.h>

class CrownstoneDimmer {

private:

	bool _initialized = false;

public:

	void init();

	void setIntensity(uint8_t intensity);

};