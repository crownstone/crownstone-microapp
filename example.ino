//
// An example to show a few functions being implemented.
//

// Show how a counter is incremented
static int counter = 100;

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
}

//
// A dummy loop function.
//
int loop() {
	// We are able to use static variables.
	counter++;

	// We can use local variables, also before and after delay() calls.
	int test = 1;

	// Do this only every 5 ticks.
	if (counter % 5 == 0) {
		Serial.write("Hi there! Greetings from the microapp!");

		// We can control a particular virtual pin.
		digitalWrite(1, 1);

		// We delay 10 seconds.
		delay(10000);

		// See protocol definition for other options.
		digitalWrite(1, 0);

		// We can also use write() and specify the number of characters explicitly.
		Serial.write("Done!", 5);

		// We increment a local variable for testing below.
		test++;

		// Print it (should print 2).
		Serial.write(test);
	}
	// Show counter.
	Serial.write(counter);
	return counter;
}
