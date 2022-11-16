#include <CrownstoneSwitch.h>

CrownstoneSwitchClass::CrownstoneSwitchClass() {}

void CrownstoneSwitchClass::turnOn(bool useBehaviour) {
	if (useBehaviour) {
		setSwitch(CS_MICROAPP_SDK_SWITCH_SMART_ON);
	}
	else {
		setSwitch(CS_MICROAPP_SDK_SWITCH_ON);
	}
}

void CrownstoneSwitchClass::turnOff() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_OFF);
}

void CrownstoneSwitchClass::dim(uint8_t percentage) {
	if (percentage > 100) {
		percentage = 100;
	}
	setSwitch(percentage);
}

void CrownstoneSwitchClass::toggle() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_TOGGLE);
}

void CrownstoneSwitchClass::asPerBehaviour() {
	setSwitch(CS_MICROAPP_SDK_SWITCH_BEHAVIOUR);
}

bool CrownstoneSwitchClass::isOn() {
	return !isOff();
}

bool CrownstoneSwitchClass::isOff() {
	auto state = getState();
	return (state.relay == false && state.dimmer == 0);
}


void CrownstoneSwitchClass::setSwitch(uint8_t val) {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(payload);
	switchRequest->header.ack            = CS_MICROAPP_SDK_ACK_REQUEST;
	switchRequest->header.messageType    = CS_MICROAPP_SDK_TYPE_SWITCH;
	switchRequest->type                  = CS_MICROAPP_SDK_SWITCH_REQUEST_SET;
	switchRequest->set                   = val;
	sendMessage();
}

microapp_sdk_switch_state_t CrownstoneSwitchClass::getState() {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(payload);
	switchRequest->header.ack            = CS_MICROAPP_SDK_ACK_REQUEST;
	switchRequest->header.messageType    = CS_MICROAPP_SDK_TYPE_SWITCH;
	switchRequest->type                  = CS_MICROAPP_SDK_SWITCH_REQUEST_GET;
	sendMessage();
	return switchRequest->get;
}
