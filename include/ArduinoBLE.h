#include <Serial.h>
#include <microapp.h>

enum BleEventHandlerType {
    BleEventDeviceScanned,
    BleEventConnected,
    BleEventDisconnected
};

enum BleFilterType {
    BleFilterNone = 0, // default
    BleFilterAddress,
    BleFilterLocalName
};

typedef struct {
    uint8_t byte[MAC_ADDRESS_LENGTH];
} MACaddress;

typedef struct {
    BleFilterType filterType;
    MACaddress address;
} BleFilter;

class Ble 
{
private:
    Ble(){};

    BleFilter _filter;

public:

    uint32_t _scanned_device_callback; // TODO: make this not accessible to users while still able to be called in handleEvent()

    static Ble & getInstance()
	{
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

    void setEventHandler(BleEventHandlerType type, void (*isr)(ble_dev_t)); // registers a callback function for some event triggered within bluenet

    bool scan(); // starts scanning for advertisements (actually starts forwarding bluenet advertisement events to registered microapp callback function in setHandler)

    void addFilter(BleFilter filter);

    BleFilter getFilter();

    //void handleEvent(ble_dev_t dev);

    //void callCallback(int arg); // temporary function to test callback function
};


#define BLE Ble::getInstance()