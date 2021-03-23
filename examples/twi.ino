// an example to interface with the MCP9808 I2C temperature sensor.

static int counter = 0;

void setup() {
	Serial.begin();
	if (!Serial) return;
	Serial.println("Setup");
	
	// join the i2c bus
	Wire.begin();
}

const byte temperature_register = 5;
const byte address_target = 0x18;

int loop() {

	// read something new every so many times
	if (counter % 5 == 0) {

		// ask the target at given address for a temperature value
		Wire.beginTransmission(address_target);
		Wire.send(temperature_register);
		Wire.endTransmission();

		// read the response (2 bytes)
		Wire.requestFrom(address_target, 2, true);
	
		byte lowByte = 0, highByte = 0, signBit = 1;

		// parse the response
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
