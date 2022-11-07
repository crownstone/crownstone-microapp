#pragma once

#include <Arduino.h>

/*
 * Class for switching the Crownstone relay and/or dimmer
 */
class CrownstoneSwitchClass {
public:
	/**
	 * Turn on the switch.
	 *
	 * @param[in] useBehaviour    Whether to let behaviour rules determine the "on" value.
	 */
	void turnOn(bool useBehaviour = true);

	/**
	 * Turn off the switch.
	 */
	void turnOff();

	/**
	 * Set a dim value: between 0 and 100%.
	 */
	void dim(uint8_t percentage);

	/**
	 * Toggle the switch.
	 *
	 * If currently off, turn on, and vice versa.
	 * The "on value" is determined by behaviour rules.
	 */
	void toggle();

	/**
	 * Set the switch value based on behaviour rules.
	 */
	void asPerBehaviour();

	/**
	 * Whether the switch is currently turned on.
	 *
	 * Any value but fully off is considered on.
	 */
	bool isOn();

	/**
	 * Whether the switch is currently turned off.
	 */
	bool isOff();

	// Class instance.
	static CrownstoneSwitchClass& getInstance() {
		// Guaranteed to be destroyed.
		static CrownstoneSwitchClass instance;

		// Instantiated on first use.
		return instance;
	}

private:
	/**
	 * Constructors and copy constructors
	 */
	CrownstoneSwitchClass();
	CrownstoneSwitchClass(CrownstoneSwitchClass const&);
	void operator=(CrownstoneSwitchClass const&);

	/**
	 * Set the switch value.
	 *
	 * Sends a message to bluenet with the switch request.
	 *
	 * @param[in] value      One of MicroappSdkSwitchValue, or a dimmed value.
	 */
	void setSwitch(uint8_t value);

	/**
	 * Get the switch state.
	 */
	microapp_sdk_switch_state_t getState();
};

#define CrownstoneSwitch CrownstoneSwitchClass::getInstance()
