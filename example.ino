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
	Serial.println("Setup started");

	// We can also write integers.
	Serial.println(counter);

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

	byte address = 0x18;

	if (counter % 5 == 0) {
		// Start transmission over i2c bus
		Wire.beginTransmission(address);
		Wire.write(5);
		Wire.endTransmission();

		Serial.print("Read req: ");

		// Request a few bytes from device at given address
		Wire.requestFrom(address, 2);

		while (Wire.available()) {
			char c = Wire.read();
			Serial.write(c);
		}
		Serial.println(".");
	}

	if (counter % 10 == 0) {
		Serial.println("Delay 10 sec");

		// We delay 10 seconds.
		delay(10000);

		// See protocol definition for other options.
		digitalWrite(1, 0);
		
		Serial.println("Done...");
	}
	// Show counter.
	Serial.println(counter);

	// Let's also advertise the latest counter value in the service data.
	serviceDataBuf[2] = counter;
	SerialServiceData.write(serviceDataBuf, sizeof(serviceDataBuf));

	return counter;
}
