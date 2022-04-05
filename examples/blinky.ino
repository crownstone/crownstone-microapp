//
// Blinking a LED
//

byte state = LOW;
const uint8_t LED3_INDEX = 0x0b;

//
// An example of a setup function.
//
void setup() {
	// Set LED3 to OUTPUT, so we can write.
	pinMode(LED3_INDEX, OUTPUT);
}

//
// A dummy loop function.
//
void loop() {
	state = !state;
	digitalWrite(LED3_INDEX, state);
}
