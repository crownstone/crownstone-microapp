//
// An example to show a few functions being implemented.
//

// Show how a counter is incremented
static int counter = 100;

//
// A dummy setup function.
//
void setup() {
	Serial.begin();

	if (!Serial) return;

	Serial.write("Setup started");

	Serial.write(counter);
}

//
// A dummy loop function.
//
int loop() {
	counter++;

	if (counter % 10 == 0) {
		Serial.write("Yes, we are looping!");
		delay(1000);
	}
	return counter;
}
