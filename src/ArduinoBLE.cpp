#include <ArduinoBLE.h>


void Ble::setEventHandler(void (*isr)(message_t))
{
    Serial.println("Setting event handler");
    //_registered_callback = isr;

    ble_cmd_t *ble_cmd = (ble_cmd_t*)&global_msg;
    ble_cmd->cmd = CS_MICROAPP_COMMAND_BLE;
    ble_cmd->opcode = CS_MICROAPP_COMMAND_BLE_SCAN_SET_HANDLER;
    ble_cmd->callback = (uint32_t)(isr); 
    global_msg.length = sizeof(ble_cmd_t);

    int success = sendMessage(&global_msg);
    
    if (success == -1)
    {
        Serial.println("Setting event handler failed");
    }
    else
    {
        Serial.println("Set event handler successfully");
    }
    
}

bool Ble::scan()
{
    Serial.println("BLE scan called");
    return true;
}

// void Ble::callCallback(int arg)
// {
//     _registered_callback(arg);
// }