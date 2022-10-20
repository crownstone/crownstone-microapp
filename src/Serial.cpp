#include <Serial.h>
#include <ipc/cs_IpcRamData.h>
#include <microapp.h>
#include <stdint.h>

// Design choice is that strings will always be null-terminated.
// The last byte will be overwritten at the bluenet side by a null byte even if this is not done in the microapp code.

void Serial_::begin() {
	// does nothing
}

microapp_size_t Serial_::write(char value) {
	return _write(value);
}

microapp_size_t Serial_::write(float value) {
	return _write(value);
}

microapp_size_t Serial_::write(double value) {
	return _write(value);
}

microapp_size_t Serial_::write(int value) {
	return _write(value);
}

microapp_size_t Serial_::write(short value) {
	return _write(value);
}

microapp_size_t Serial_::write(unsigned int value) {
	return _write(value);
}

microapp_size_t Serial_::write(const char* str) {
	return _write(str);
}

microapp_size_t Serial_::write(String str) {
	return _write(str);
}

microapp_size_t Serial_::write(const uint8_t* buf, int length) {
	return _write(buf, length);
}

// Copies of the above functions

microapp_size_t Serial_::print(char value) {
	return _write(value);
}

microapp_size_t Serial_::print(float value) {
	return _write(value);
}

microapp_size_t Serial_::print(double value) {
	return _write(value);
}

microapp_size_t Serial_::print(int value) {
	return _write(value);
}

microapp_size_t Serial_::print(short value) {
	return _write(value);
}

microapp_size_t Serial_::print(unsigned int value) {
	return _write(value);
}

microapp_size_t Serial_::print(const char* str) {
	return _write(str);
}

microapp_size_t Serial_::print(String str) {
	return _write(str);
}

microapp_size_t Serial_::print(const uint8_t* buf, int length) {
	return _write(buf, length);
}

// Copies of the above functions, but with newline option enabled

microapp_size_t Serial_::println(char value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(float value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(double value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(int value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(short value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(unsigned int value) {
	return _write(value, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(const char* str) {
	return _write(str, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(String str) {
	return _write(str, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

microapp_size_t Serial_::println(const uint8_t* buf, int length) {
	return _write(buf, length, CS_MICROAPP_SDK_LOG_FLAG_NEWLINE);
}

/// Implementations (protected)

microapp_size_t Serial_::_write(char value, MicroappSdkLogFlags flags) {
	uint8_t* payload                    = getOutgoingMessagePayload();
	microapp_sdk_log_char_t* logRequest = reinterpret_cast<microapp_sdk_log_char_t*>(payload);
	logRequest->value                   = value;
	logRequest->logHeader.size          = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Char, flags);
}

microapp_size_t Serial_::_write(float value, MicroappSdkLogFlags flags) {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_log_float_t* logRequest = reinterpret_cast<microapp_sdk_log_float_t*>(payload);
	logRequest->value                    = value;
	logRequest->logHeader.size           = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Float, flags);
}

microapp_size_t Serial_::_write(double value, MicroappSdkLogFlags flags) {
	uint8_t* payload                      = getOutgoingMessagePayload();
	microapp_sdk_log_double_t* logRequest = reinterpret_cast<microapp_sdk_log_double_t*>(payload);
	logRequest->value                     = value;
	logRequest->logHeader.size            = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Double, flags);
}

microapp_size_t Serial_::_write(int value, MicroappSdkLogFlags flags) {
	uint8_t* payload                   = getOutgoingMessagePayload();
	microapp_sdk_log_int_t* logRequest = reinterpret_cast<microapp_sdk_log_int_t*>(payload);
	logRequest->value                  = value;
	logRequest->logHeader.size         = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Int, flags);
}

microapp_size_t Serial_::_write(short value, MicroappSdkLogFlags flags) {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_log_short_t* logRequest = reinterpret_cast<microapp_sdk_log_short_t*>(payload);
	logRequest->value                    = value;
	logRequest->logHeader.size           = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Short, flags);
}

microapp_size_t Serial_::_write(unsigned int value, MicroappSdkLogFlags flags) {
	uint8_t* payload                    = getOutgoingMessagePayload();
	microapp_sdk_log_uint_t* logRequest = reinterpret_cast<microapp_sdk_log_uint_t*>(payload);
	logRequest->value                   = value;
	logRequest->logHeader.size          = sizeof(value);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::UnsignedInt, flags);
}

microapp_size_t Serial_::_write(const char* str, MicroappSdkLogFlags flags) {
	uint8_t* payload                      = getOutgoingMessagePayload();
	microapp_sdk_log_string_t* logRequest = reinterpret_cast<microapp_sdk_log_string_t*>(payload);
	if (strlen(str) > MAX_STRING_SIZE) {
		logRequest->logHeader.size = MAX_STRING_SIZE;
	}
	else {
		logRequest->logHeader.size = strlen(str);
	}
	memcpy(logRequest->str, str, logRequest->logHeader.size);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Str, flags);
}

microapp_size_t Serial_::_write(String str, MicroappSdkLogFlags flags) {
	uint8_t* payload                      = getOutgoingMessagePayload();
	microapp_sdk_log_string_t* logRequest = reinterpret_cast<microapp_sdk_log_string_t*>(payload);
	if (str.length() > MAX_STRING_SIZE) {
		logRequest->logHeader.size = MAX_STRING_SIZE;
	}
	else {
		logRequest->logHeader.size = str.length();
	}
	memcpy(logRequest->str, str.c_str(), logRequest->logHeader.size);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Str, flags);
}

microapp_size_t Serial_::_write(const uint8_t* buf, int length, MicroappSdkLogFlags flags) {
	uint8_t* payload                     = getOutgoingMessagePayload();
	microapp_sdk_log_array_t* logRequest = reinterpret_cast<microapp_sdk_log_array_t*>(payload);
	if (length < 0) {
		return 0;
	}
	if (length > MICROAPP_SDK_MAX_ARRAY_SIZE) {
		logRequest->logHeader.size = MICROAPP_SDK_MAX_ARRAY_SIZE;
	}
	else {
		logRequest->logHeader.size = length;
	}
	memcpy(logRequest->arr, buf, logRequest->logHeader.size);
	return _write(reinterpret_cast<microapp_sdk_log_header_t*>(logRequest), Type::Arr, flags);
}

//
// Write over serial. We will try to write if possible and return as few possible errors as possible.
// For example if the string is too long, we will truncate it and return only the first portion rather
// than silently fail.
//
microapp_size_t Serial_::_write(microapp_sdk_log_header_t* logRequest, Type type, MicroappSdkLogFlags flags) {
	logRequest->header.messageType = CS_MICROAPP_SDK_TYPE_LOG;
	logRequest->header.ack         = CS_MICROAPP_SDK_ACK_REQUEST;
	logRequest->type               = type;
	logRequest->flags              = flags;
	sendMessage();
	return logRequest->size;
}
