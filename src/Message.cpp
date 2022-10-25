/**
 * Message.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Oct 21, 2022
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <Message.h>
#include <stdint.h>

MessageClass::MessageClass() {

}

microapp_sdk_result_t handleMessageInterrupt(void* interrupt) {
	return Message.handleInterrupt(interrupt);
}

bool MessageClass::begin() {
	if (_registeredInterrupt) {
		return true;
	}

	// Register interrupt on the microapp side,
	// with an indirect handler
	interrupt_registration_t interrupt;
	interrupt.type               = CS_MICROAPP_SDK_TYPE_MESSAGE;
	interrupt.id                 = 0;
	interrupt.handler            = handleMessageInterrupt;
	microapp_sdk_result_t result = registerInterrupt(&interrupt);
	if (result != CS_MICROAPP_SDK_ACK_SUCCESS) {
		// No empty interrupt slots available on microapp side
		return false;
	}

	// Register an interrupt on the bluenet side.
	uint8_t* payload            = getOutgoingMessagePayload();
	auto request                = (microapp_sdk_message_t*)(payload);
	request->header.messageType = CS_MICROAPP_SDK_TYPE_MESSAGE;
	request->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	request->type               = CS_MICROAPP_SDK_MSG_REGISTER_INTERRUPT;

	if (sendMessage() != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return false;
	}

	_registeredInterrupt = true;
	return true;
}

microapp_sdk_result_t MessageClass::handleInterrupt(void* interrupt) {
	auto message = reinterpret_cast<microapp_sdk_message_t*>(interrupt);
	switch (message->type) {
		case CS_MICROAPP_SDK_MSG_EVENT_RECEIVED_MSG: {
			if (_handler != nullptr) {
				(*_handler)(message->receivedMessage.data, message->receivedMessage.size);
				return CS_MICROAPP_SDK_ACK_SUCCESS;
			}

			size_t totalSize = _available + message->receivedMessage.size;
			if (totalSize > sizeof(_receiveBuffer)) {
				// This won't fit in the buffer.
				return CS_MICROAPP_SDK_ACK_ERR_NO_SPACE;
			}

			memcpy(_receiveBuffer + _available, message->receivedMessage.data, message->receivedMessage.size);
			_available += message->receivedMessage.size;
			return CS_MICROAPP_SDK_ACK_SUCCESS;
		}
		default: {
			return CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
		}
	}
}

microapp_size_t MessageClass::available() {
	return _available;
}

microapp_size_t MessageClass::readBytes(void* data, microapp_size_t size) {
	if (_available < size) {
		size = _available;
	}

	// Copy data to the buffer.
	memcpy(data, _receiveBuffer, size);

	// Shift the data in the buffer, in case there is more available.
	for (int i = 0; i < _available - size; ++i) {
		_receiveBuffer[i] = _receiveBuffer[i + size];
	}

	_available -= size;
	return size;
}


microapp_size_t MessageClass::write(void* data, microapp_size_t size) {
	uint8_t* payload                = getOutgoingMessagePayload();
	microapp_sdk_message_t* request = reinterpret_cast<microapp_sdk_message_t*>(payload);
	request->header.messageType     = MicroappSdkType::CS_MICROAPP_SDK_TYPE_MESSAGE;
	request->header.ack             = CS_MICROAPP_SDK_ACK_REQUEST;
	request->type                   = CS_MICROAPP_SDK_MSG_REQUEST_SEND_MSG;

	// Clip the size.
	if (size > MICROAPP_SDK_MESSAGE_SEND_MSG_MAX_SIZE) {
		size = MICROAPP_SDK_MESSAGE_SEND_MSG_MAX_SIZE;
	}
	request->sendMessage.size = size;
	memcpy(request->sendMessage.data, data, size);

	if (sendMessage() != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return 0;
	}
	return size;
}

bool MessageClass::setHandler(MessageHandler handler) {
	_handler = handler;
	return true;
}
