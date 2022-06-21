#include <Arduino.h>

boolean state = LOW;

void blink() {
	Serial.println("blink");

	// Toggle LED
	state = !state;
	digitalWrite(LED1_PIN, state);
}

void setup() {
	Serial.begin();

	// Configure LED pin as output
	pinMode(LED1_PIN, OUTPUT);

	// Set interrupt handler
	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	if (attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), blink, RISING) < 0) {
		Serial.println("Setting button interrupt failed");
	}

}

void loop() {

}
