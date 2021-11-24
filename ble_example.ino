#include <ArduinoBLE.h>

// A ble microapp example for reading advertisements from a Xiaomi Thermometer with custom firmware: https://github.com/atc1441/ATC_MiThermometer


void my_callback_func(ble_dev_t dev) // callback for received peripheral advertisement
{
    Serial.println("my_callback_func: ");
    Serial.print("\trssi: "); Serial.println(dev.rssi);
    Serial.print("\taddress type: "); Serial.println(dev.addr_type);
    Serial.print("\taddress: "); Serial.println(dev.addr,sizeof(dev.addr));
    Serial.print("\tdlen: "); Serial.println(dev.dlen);
    Serial.print("\tdata: "); Serial.println(dev.data,dev.dlen);

    // parse advertisement data
    uint8_t adType = dev.data[1];
    switch(adType) {
        case 0x09 : {
            Serial.print("\tComplete local name: "); 
            char* localName = (char*) &(dev.data[2]);
            Serial.println(localName,dev.dlen-2);
            break;
        }
        case 0x16 : {
            Serial.println("\tService data");
            uint8_t UUID[2] = {dev.data[3], dev.data[2]};
            Serial.print("\t\tUUID: "); Serial.println(UUID,2);
            uint16_t temperature = (dev.data[10] << 8) | dev.data[11];
            Serial.print("\t\tTemperature: "); Serial.println(temperature);
            uint8_t humidity = dev.data[12];
            Serial.print("\t\tHumidity: "); Serial.println(humidity);
            uint16_t battery_perc = dev.data[13];
            Serial.print("\t\tBattery \%: "); Serial.println(battery_perc);
        }
    }
}

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Setup");

    // Set scanning filters
    MACaddress mac = {0xA4, 0xC1, 0x38, 0x9A, 0x45, 0xE3}; // We are looking for the BLE sensor with MAC address A4:C1:38:9A:45:E3
    BleFilter filter; filter.filterType = BleFilterAddress; filter.address = mac;
    BLE.addFilter(filter);

    // Register my_callback_func
    BLE.setEventHandler(BleEventDeviceScanned, my_callback_func);

    // Start scanning for BLE ads
    BLE.scan();
}

// The Arduino loop function.
int loop() {

	// Say something every time we loop (which is every second)
	Serial.println("Loop");

    return 1;

}