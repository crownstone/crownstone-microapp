// A microapp example that prints hello world.

// The Arduino setup function.
void setup() {
	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Write something to the log (will be shown in the bluenet code as print statement).
	Serial.println("Hello world");
}

// The Arduino loop function.
int loop() {
	// Say high ever time we loop (which is every second)
	Serial.println("Hi!");
}
