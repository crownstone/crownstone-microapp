#pragma once

#include <Arduino.h>

class CrownstoneRelay {

private:

	bool _initialized = false;

	void setSwitch(MicroappSdkSwitchValue val);

public:

	void init();

	void switchOff();

	void switchOn();

	void switchToggle();

	void switchBehaviour();

	void switchSmartOn();

};