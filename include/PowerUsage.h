#pragma once

#include <microapp.h>

class PowerUsageClass {
public:
	/**
	 * Get the power usage in milliwatt.
	 */
	int32_t getPowerUsageMilliWatts();

	// Class instance.
	static PowerUsageClass& getInstance() {
		// Guaranteed to be destroyed.
		static PowerUsageClass instance;

		// Instantiated on first use.
		return instance;
	}

private:
	/**
	 * Constructors and copy constructors
	 */
	PowerUsageClass();
	PowerUsageClass(PowerUsageClass const&);
	void operator=(PowerUsageClass const&);
};

#define PowerUsage PowerUsageClass::getInstance()
