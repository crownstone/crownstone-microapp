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
	Serial.println("Setup");

	// Join the i2c bus
	Wire.begin();

	/*
	byte address = 0x18;
	byte control = 0x01;
	byte val = 0x00;
	// Configure
	Wire.beginTransmission(address);
	Wire.send(control);
	// Continuous conversion mode, Power-up default
	Wire.send(val);
	Wire.send(val);
	Wire.endTransmission();
	*/
}

const byte TEMPERATURE_REGISTER = 5;

// Loop every second
int loop() {

	byte address = 0x18;

	// Read something new every so many times
	if (counter % 5 == 0) {
		// register 5
		Wire.beginTransmission(address);
		Wire.send(TEMPERATURE_REGISTER);
		Wire.endTransmission();

		// Request 2 bytes from device at given address
		Wire.requestFrom(address, 2, true);
	
		byte lowByte = 0, highByte = 0;
		byte signBit = 1;
		if (Wire.available() > 1) {
			// make sure highByte is limited to 5 bits (total 13)
			highByte = Wire.read() & 0x1F;
			// the 13th bit (5th bit of high byte) is the sign bit
			if (highByte & 0x10) {
				signBit = -1;
			}
			// remove sign bit
			highByte &= 0x0F;
			lowByte = Wire.read();
		}

		short value = word(highByte, lowByte);

		// default resolution
		float temp = signBit * value * 0.0625;
		Serial.print("Temperature in Celsius: ");
		Serial.print(temp);
		Serial.println(" °C");
	}

	counter++;

	return counter;
}
