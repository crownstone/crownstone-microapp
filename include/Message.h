/*
 * Message.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Oct 21, 2022
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <microapp.h>
#include <stdint.h>

/**
 * Handle an incoming data message.
 */
typedef void (*MessageHandler)(uint8_t* data, microapp_size_t size);

/**
 * Class to send data messages to uart, and receive data messages from control command.
 */
class MessageClass {
public:
	static MessageClass& getInstance() {
		// Guaranteed to be destroyed.
		static MessageClass instance;

		// Instantiated on first use.
		return instance;
	}

	/**
	 * Start the class.
	 *
	 * Registers a handler for incoming data messages.
	 */
	bool begin();

	/**
	 * Send data.
	 *
	 * @param[in] data       Data to write.
	 * @param[in] size       Size of the data in bytes.
	 *
	 * @return               Actual number of bytes that have been written.
	 */
	microapp_size_t write(void* data, microapp_size_t size);

	/**
	 * Returns number of bytes available to read.
	 */
	microapp_size_t available();

	/**
	 * Read bytes.
	 * Blocks until the bytes have been read, or until a timeout.
	 *
	 * @param[in] data       Buffer to read to.
	 * @param[in] size       Number of bytes to read, the buffer must be at least of this size.
	 *
	 * @return  Actual number of bytes read.
	 */
	microapp_size_t readBytes(void* data, microapp_size_t size);

	/**
	 * Set a message handler.
	 *
	 * When set, this replaces available() and readBytes().
	 */
	bool setHandler(MessageHandler& handler);

private:
	MessageClass();
	MessageClass(MessageClass const&)   = delete;
	void operator=(MessageClass const&) = delete;

	//! Whether the interrupt handler has been registered.
	bool _registeredInterrupt = false;

	//! Buffer for received messages.
	uint8_t _receiveBuffer[MICROAPP_SDK_MESSAGE_RECEIVED_MSG_MAX_SIZE];

	//! Keeps up number of bytes available to read.
	microapp_size_t _available = 0;

	//! The message handler.
	MessageHandler* _handler = nullptr;

	//! Handle an interrupt.
	microapp_sdk_result_t handleInterrupt(void* interrupt);

	//! Give access to private functions to this function.
	friend microapp_sdk_result_t handleInterruptStatic(void*);
};

//! The global instance.
#define Message MessageClass::getInstance()
