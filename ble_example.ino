#include <ArduinoBLE.h>

// A ble microapp example.

void my_callback_func(message_t msg) // callback for received peripheral advertisement
{
    ble_adv_t *adv = (ble_adv_t*)&msg; // recast. TODO: return a BleDevice object
    Serial.print("my_callback_func: ");
    Serial.print("\trssi: "); Serial.println(adv->rssi);
    Serial.print("\ttype: "); Serial.println(adv->type);
}

int cntr = 0;

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Setup");

    // Register my_callback_func
    BLE.setEventHandler(my_callback_func);

    // Start scanning for BLE ads
    BLE.scan();

}

// The Arduino loop function.
int loop() {

    cntr++;

	// Say something ever time we loop (which is every second)
	Serial.println("Loop");

    //BLE.callCallback(cntr); // to test the callback function

    return 1;

}