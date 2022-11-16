#include <PowerUsage.h>

PowerUsageClass::PowerUsageClass() {}

int32_t PowerUsageClass::getPowerUsageMilliWatts() {
	uint8_t* out                             = getOutgoingMessagePayload();
	microapp_sdk_power_usage_t* powerRequest = reinterpret_cast<microapp_sdk_power_usage_t*>(out);
	powerRequest->header.messageType         = CS_MICROAPP_SDK_TYPE_POWER_USAGE;
	powerRequest->header.ack                 = CS_MICROAPP_SDK_ACK_REQUEST;
	powerRequest->type                       = CS_MICROAPP_SDK_POWER_USAGE_POWER;
	sendMessage();
	if (powerRequest->header.ack != CS_MICROAPP_SDK_ACK_SUCCESS) {
		return -1;
	}
	return powerRequest->powerUsage;
}
