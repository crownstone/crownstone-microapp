/*
 * BluenetInternal.
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
 * Handle an internal bluenet event.
 *
 * @param[in] bluenetType    The bluenet type, can be found in cs_Types.h.
 * @param[in] data           The data of this event, the data structure is different per type.
 * @param[in] size           The size of the data.
 */
typedef void (*BluenetEventHandler)(uint16_t bluenetType, uint8_t* data, microapp_size_t size);

/**
 * Class to access bluenet internals.
 *
 * Currently only register for bluenet events.
 */
class BluenetInternalClass {
public:
	static BluenetInternalClass& getInstance() {
		// Guaranteed to be destroyed.
		static BluenetInternalClass instance;

		// Instantiated on first use.
		return instance;
	}

	static const uint8_t MAX_SUBSCRIPTIONS = 10;

	/**
	 * Registers a callback function for bluenet events.
	 *
	 * @param[in] eventHandler    The event handler.
	 * @return                    True on success
	 */
	bool setEventHandler(BluenetEventHandler& eventHandler);

	/**
	 * Subscribe for a bluenet event type.
	 *
	 * @param[in] bluenetType     The type to subscribe for.
	 * @return                    True on success
	 */
	bool subscribe(uint16_t bluenetType);

private:
	BluenetInternalClass();
	BluenetInternalClass(BluenetInternalClass const&)   = delete;
	void operator=(BluenetInternalClass const&) = delete;

	bool _registeredInterrupt = false;

	/**
	 * Keep up the types we are subscribed to.
	 * 0 for empty.
	 */
	uint16_t _subscribedTypes[MAX_SUBSCRIPTIONS];

	//! Keep up the registered event handler.
	BluenetEventHandler* _eventHandler = nullptr;

	//! Handle an interrupt.
	microapp_sdk_result_t handleInterrupt(void* interrupt);

	//! Give access to private functions to this function.
	friend microapp_sdk_result_t handleBluenetInternalInterrupt(void*);
};

//! The global instance.
#define BluenetInternal BluenetInternalClass::getInstance()
