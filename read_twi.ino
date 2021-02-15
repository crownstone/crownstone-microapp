//
// An example to interface with an twi/i2c sensor 
//

// Show how a counter is incremented
static int counter = 0;

// Setting up everything.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.write("Setup");

	// Join the i2c bus
	Wire.begin();
}

// Loop every second
int loop() {

	byte address = 0x18;

	// Show we are there
	if (counter % 20 == 0) {
		Serial.write("Hi");
	}

	// Show counter.
	Serial.write(counter);

	// Read something new every so many times
	if (counter % 5 == 0) {
		Serial.write("i2c");

		Wire.beginTransmission(address);
		Wire.send(5);
		Wire.endTransmission();

		// Request 2 bytes from device at given address
		Wire.requestFrom(address, 2, true);

		while (Wire.available()) {
			char c = Wire.read();
			Serial.write(c);
		}
	}

	counter++;

	return counter;
}
