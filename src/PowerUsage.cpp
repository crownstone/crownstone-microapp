#include <PowerUsage.h>

int32_t PowerUsage::getPowerUsageInMilliWatts() {

	global_msg.payload[0] = CS_MICROAPP_COMMAND_POWER_USAGE;

	microapp_power_usage_t* cmd_payload = (microapp_power_usage_t*)&global_msg.payload[1];
	cmd_payload->powerUsage             = 0;

	global_msg.length = 1 + sizeof(microapp_power_usage_t);

	sendMessage(&global_msg);

	return cmd_payload->powerUsage;
}
