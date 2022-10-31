#include <Arduino.h>

boolean state = LOW;

uint16_t counter = 0;

void onPushedButton() {
	state = !state;
	digitalWrite(LED1_PIN, state);
	// Async call in interrupt handler
	delay(5000);
}

void setup() {
	Serial.println("Nested interrupt test");

	pinMode(LED1_PIN, OUTPUT);

	pinMode(BUTTON1_PIN, INPUT_PULLUP);
	if (!attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), onPushedButton, RISING)) {
		Serial.println("Setting button 1 interrupt failed");
	}
	digitalWrite(LED1_PIN, state);
}

void loop() {
	Serial.println(counter++);
}
