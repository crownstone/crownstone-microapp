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
    BleFilterLocalName,
    BleFilterServiceData
};

typedef struct {
    uint8_t byte[MAC_ADDRESS_LENGTH];
} MACaddress;

typedef struct {
    BleFilterType filterType;
    MACaddress address;
    const char* completeLocalName;
    uint16_t uuid;
} BleFilter;

class Ble 
{
private:
    Ble(){};

    BleFilter _filter;

    bool _isScanning = false;

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

    void stopScan(); // stops calling the registered microapp callback upon scanned bluenet advertisements

    bool isScanning();

    void addFilter(BleFilter filter);

    void removeFilter();

    BleFilter getFilter();

};


#define BLE Ble::getInstance()