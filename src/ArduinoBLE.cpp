#include <ArduinoBLE.h>

// Filters and forwards the bluenet scanned device event interrupt to the user callback
void handleScanEvent(ble_dev_t dev)
{
    BleFilter filter = BLE.getFilter();
    switch (filter.filterType) {
        case BleFilterAddress:{
            //Serial.print("Scanned device MAC address "); Serial.println(dev.addr, sizeof(dev.addr));
            if (!memcmp(dev.addr,filter.address.byte,MAC_ADDRESS_LENGTH)) return;
        }
        case BleFilterLocalName:
        case BleFilterNone:
        default:
            break;
    }
    void (*callback_func)(ble_dev_t) = (void (*)(ble_dev_t)) BLE._scanned_device_callback;
    callback_func(dev);
}

void Ble::setEventHandler(BleEventHandlerType type, void (*isr)(ble_dev_t))
{
    Serial.println("Setting event handler");

    //TODO: add switch to set different handlers based on type

    ble_cmd_t *ble_cmd = (ble_cmd_t*)&global_msg;
    ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
    ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;
    ble_cmd->callback = (uint32_t)(handleScanEvent);
    _scanned_device_callback = (uint32_t)(isr);

    global_msg.length = sizeof(ble_cmd_t);

    sendMessage(&global_msg);
}

bool Ble::scan()
{
    Serial.println("BLE scan called");
    return true;
}

void Ble::addFilter(BleFilter filter)
{
    Serial.print("Setting filter");
    _filter = filter;
}

BleFilter Ble::getFilter()
{
    return _filter;
}
