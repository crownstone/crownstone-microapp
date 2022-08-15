#include <CrownstoneRelay.h>
#include <Serial.h>

void CrownstoneRelay::init() {
	_initialized = true;
}

void CrownstoneRelay::switchOff() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_OFF);
}

void CrownstoneRelay::switchOn() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_ON);
}

void CrownstoneRelay::switchToggle() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_TOGGLE);
}

void CrownstoneRelay::switchBehaviour() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_BEHAVIOUR);
}

void CrownstoneRelay::switchSmartOn() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_SMART_ON);
}

void CrownstoneRelay::setSwitch(MicroappSdkSwitchValue val) {
	if (!_initialized) {
		Serial.println("Relay not initialized");
		return;
	}
	uint8_t *payload = getOutgoingMessagePayload();
	microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(payload);
	switchRequest->header.sdkType        = CS_MICROAPP_SDK_TYPE_SWITCH;
	switchRequest->value                 = val;

	sendMessage();
}