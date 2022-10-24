/**
 * BluenetInternal.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Oct 21, 2022
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <BluenetInternal.h>
#include <stdint.h>

BluenetInternalClass::BluenetInternalClass() {
	for (unsigned int i = 0; i < MAX_SUBSCRIPTIONS; ++i) {
		_subscribedTypes[i] = 0;
	}
}

microapp_sdk_result_t handleBluenetInternalInterrupt(void* interrupt) {
	return BluenetInternal.handleInterrupt(interrupt);
}

microapp_sdk_result_t BluenetInternalClass::handleInterrupt(void* interrupt) {
	auto message = reinterpret_cast<microapp_sdk_bluenet_event_t*>(interrupt);
	switch (message->type) {
		case CS_MICROAPP_SDK_BLUENET_EVENT_EVENT: {
			if (_eventHandler != nullptr) {
				(*_eventHandler)(message->eventType, message->event.data, message->event.size);
			}
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

bool BluenetInternalClass::setEventHandler(BluenetEventHandler& eventHandler) {
	if (!_registeredInterrupt) {
		// Register interrupt on the microapp side,
		// with an indirect handler
		interrupt_registration_t interrupt;
		interrupt.type               = CS_MICROAPP_SDK_TYPE_BLUENET_EVENT;
		interrupt.id                 = 0;
		interrupt.handler            = handleBluenetInternalInterrupt;
		microapp_sdk_result_t result = registerInterrupt(&interrupt);
		if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
			// No empty interrupt slots available on microapp side
			return false;
		}
		_registeredInterrupt = true;
	}

	_eventHandler = &eventHandler;
	return true;
}

bool BluenetInternalClass::subscribe(uint16_t bluenetType) {
	if (bluenetType == 0) {
		return false;
	}

	for (unsigned int i = 0; i < MAX_SUBSCRIPTIONS; ++i) {
		if (_subscribedTypes[i] == bluenetType) {
			// Already subscribed.
			return true;
		}
	}

	for (unsigned int i = 0; i < MAX_SUBSCRIPTIONS; ++i) {
		if (_subscribedTypes[i] == 0) {
			// Empty slot: subscribe for new type.

			// Register an interrupt on the bluenet side.
			uint8_t* payload            = getOutgoingMessagePayload();
			auto request                = (microapp_sdk_bluenet_event_t*)(payload);
			request->header.messageType = CS_MICROAPP_SDK_TYPE_BLUENET_EVENT;
			request->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
			request->type               = CS_MICROAPP_SDK_BLUENET_EVENT_REGISTER_INTERRUPT;
			request->eventType          = bluenetType;

			if (sendMessage() != CS_MICROAPP_SDK_ACK_SUCCESS) {
				return false;
			}

			_subscribedTypes[i] = bluenetType;
			return true;
		}
	}

	// No space
	return false;
}
