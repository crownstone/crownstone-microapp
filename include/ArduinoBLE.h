#include <Serial.h>
#include <microapp.h>

//typedef void (*callback)(int); // typedef return_type (*function_type)(arg_type)

class Ble 
{
private:
    Ble(){};

    //callback _registered_callback; // temporary since callback will be stored and called on bluenet side

public:
    static Ble & getInstance()
	{
		// Guaranteed to be destroyed.
		static Ble instance;

		// Instantiated on first use.
		return instance;
	}

    void setEventHandler(void (*isr)(message_t)); // registers a callback function for some event triggered within bluenet

    bool scan(); // starts scanning for advertisements (actually starts forwarding bluenet advertisement events to registered microapp callback function in setHandler)

    //void callCallback(int arg); // temporary function to test callback function
};


#define BLE Ble::getInstance()