#include <Serial.h>
#include <microapp.h>
#include <BleUtils.h>
#include <BleDevice.h>

enum BleEventType {
	BleEventDeviceScanned,
	BleEventConnected,
	BleEventDisconnected
};

enum BleFilterType {
	BleFilterNone = 0, // default
	BleFilterAddress,
	BleFilterLocalName,
	BleFilterUuid
};

struct BleFilter {
	BleFilterType type;
	union {
		MACaddress address;
		char name[MAX_BLE_ADV_DATA_LENGTH]; // max length of name equals max advertisement length
		uuid16_t uuid;
	};
	uint16_t len; // length of the name
};


class Ble {
private:
	Ble(){};

	BleDevice _bleDev;

	BleFilter _activeFilter;

	bool _isScanning = false;

	uintptr_t _scanned_device_callback;

	/*
	 * Add handleScanEventWrapper as a friend so it can access private function handleScanEvent of Ble
	 */
	friend void handleScanEventWrapper(microapp_ble_dev_t dev);

	/*
	 * Handler for scanned devices. Called from bluenet via handleScanEventWrapper upon scanned device events if scanning
	 */
	void handleScanEvent(microapp_ble_dev_t dev);

	/*
	 * Compares the scanned device dev against the filter and returns true upon a match
	 */
	bool filterScanEvent(BleDevice dev);

public:

	static Ble & getInstance() {
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

	/*
	 * Registers a callback function for scanned device event triggered within bluenet
	 */
	void setEventHandler(BleEventType eventType, void (*isr)(BleDevice));

	/*
	 * Sends command to bluenet to call registered microapp callback function upon receiving advertisements
	 */
	bool scan(bool withDuplicates = false);

	/*
	 * Registers filter with name name and calls scan()
	 */
	bool scanForName(const char* name, bool withDuplicates = false);

	/*
	 * Registers filter with MAC address address and calls scan()
	 */
	bool scanForAddress(const char* address, bool withDuplicates = false);

	/*
	 * Registers filter with service data uuid uuid and calls scan()
	 */
	bool scanForUuid(const char* uuid, bool withDuplicates = false);

	/*
	 * Sends command to bluenet to stop calling registered microapp callback function upon receiving advertisements
	 */
	void stopScan();

	/*
	 * Returns a pointer to the currently set filter for scanned devices
	 */
	BleFilter* getFilter();

};


#define BLE Ble::getInstance()