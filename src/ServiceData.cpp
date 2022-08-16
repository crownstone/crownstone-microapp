#include <ServiceData.h>

void ServiceData_::write(uint16_t appUuid, const char* str) {
    uint8_t *payload = getOutgoingMessagePayload();
    microapp_sdk_service_data_t* serviceData = reinterpret_cast<microapp_sdk_service_data_t*>(payload);
    serviceData->appUuid = appUuid;
    size_t size = strlen(str);
    // Silently truncate data if too long
    if (size > MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH) {
        serviceData->size = MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH;
    }
    else {
        serviceData->size = size;
    }
    memcpy(serviceData->data, str, serviceData->size);
    _write(serviceData);
}

void ServiceData_::write(uint16_t appUuid, String str) {
    uint8_t *payload = getOutgoingMessagePayload();
    microapp_sdk_service_data_t* serviceData = reinterpret_cast<microapp_sdk_service_data_t*>(payload);
    serviceData->appUuid = appUuid;
    // Silently truncate data if too long
    if (str.length() > MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH) {
        serviceData->size = MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH;
    }
    else {
        serviceData->size = str.length();
    }
    memcpy(serviceData->data, str.c_str(), serviceData->size);
    _write(serviceData);
}

void ServiceData_::write(uint16_t appUuid, uint8_t* buf, size_t size) {
    uint8_t *payload = getOutgoingMessagePayload();
    microapp_sdk_service_data_t* serviceData = reinterpret_cast<microapp_sdk_service_data_t*>(payload);
    serviceData->appUuid = appUuid;
    // Silently truncate data if too long
    if (size > MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH) {
        serviceData->size = MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH;
    }
    else {
        serviceData->size = size;
    }
    memcpy(serviceData->data, buf, serviceData->size);
    _write(serviceData);
}

// Set header fields and call sendMessage
void ServiceData_::_write(microapp_sdk_service_data_t* serviceData) {
    serviceData->header.sdkType = CS_MICROAPP_SDK_TYPE_SERVICE_DATA;
    serviceData->header.ack = CS_ACK_REQUEST;
    sendMessage();
}