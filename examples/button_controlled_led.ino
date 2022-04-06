#include <Arduino.h>

boolean state = LOW;

void blink() {
	Serial.println("blink");

	// Toggle LED
	state = !state;
	digitalWrite(LED1_PIN, state);
}

void setup() {

	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Configure LED pin as output
	pinMode(LED1_PIN, OUTPUT);

	// Set interrupt handler
	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	int result = attachInterrupt(BUTTON2_PIN, blink, RISING);

	Serial.print("attachInterrupt returns ");
	Serial.println(result);
}

void loop() {

}