#pragma once

#include <Arduino.h>

class CrownstoneRelay {

private:

	bool _initialized = false;

	void setSwitch(CommandMicroappSwitchValue val);

public:

	void init();

	void switchOff();

	void switchOn();

	void switchToggle();

};