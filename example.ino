//
// An example to show a few functions being implemented.
//

// Show how a counter is incremented
static int counter = 100;

uint8_t serviceDataBuf[12] = {0};

//
// A dummy setup function.
//
void setup() {
	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.write("Setup started");

	// We can also write integers.
	Serial.write(counter);

	// Set the UUID of this microapp.
	serviceDataBuf[0] = 12;
	serviceDataBuf[1] = 34;

	// Set digital port 1 to OUTPUT, so we can write.
	pinMode(1, OUTPUT);

	// Join the i2c bus
	Wire.begin();
}

//
// A dummy loop function.
//
int loop() {
	// We are able to use static variables.
	counter++;

	// We can use local variables, also before and after delay() calls.
	// int test = 1;

	byte address = 0x7;
		
	// Start transmission over i2c bus
	Wire.beginTransmission(address);
	Wire.write("i2c");
	Wire.endTransmission();

	if (counter == 101) {
		Serial.write("Read req");

		// Request 5 bytes from device at given address
		Wire.requestFrom(address, 5);

		while (Wire.available()) {
			char c = Wire.read();
			Serial.write(c);
		}
	}

	// Do this only every 5 ticks.
	if (counter % 5 == 0) {
//		Serial.write("Hi there! Greetings from the microapp!");

		// We can control a particular virtual pin.
//		digitalWrite(1, 1);

		// We delay 10 seconds.
		delay(10000);

		// See protocol definition for other options.
		digitalWrite(1, 0);
		
		Serial.write("Done!");

		// We increment a local variable for testing below.
		//test++;

		// Print it (should print 2).
		// Serial.write(test);
	}
	// Show counter.
	Serial.write(counter);

	// Let's also advertise the latest counter value in the service data.
	serviceDataBuf[2] = counter;
	SerialServiceData.write(serviceDataBuf, sizeof(serviceDataBuf));

	return counter;
}
