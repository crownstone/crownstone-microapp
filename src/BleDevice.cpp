#include <BleDevice.h>

String BleDevice::address() {
	MACaddress mac;
	memcpy(mac.byte,_dev->addr,MAC_ADDRESS_LENGTH);
	convertMacToString(mac,_address_str);
	return _address_str;
}