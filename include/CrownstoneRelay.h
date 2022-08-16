#pragma once

#include <Arduino.h>

/*
 * Class for switching the crownstone relay
 */
class CrownstoneRelay {
private:
	// Set the switch value. Sends a message to bluenet with the switch request
	void setSwitch(MicroappSdkSwitchValue val);

public:
	// Turn off the switch
	void switchOff();

	// Turn on the switch
	void switchOn();

	// Toggle the switch. If currently off, turn to smart on
	void switchToggle();

	// Switch to value based on behaviour rules
	void switchBehaviour();

	// Switch on, the value will be determined by behaviour rules
	void switchSmartOn();
};