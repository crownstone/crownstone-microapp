//
// Microapp for smart outlet.
//
// We check for events from BUTTON2.
// We turn on GPIO0
// An example to show a few functions being implemented.
//

#include <Arduino.h>

volatile byte state = LOW;
volatile byte state2 = LOW;
volatile byte state3 = LOW;

// The button_pressed function will be called on interrupt CHANGE. This means when you press and when you release a
// button.
void button_pressed() {
	state3 = digitalRead(BUTTON2_PIN);
	if (state3 == HIGH) {
		state = HIGH;
		Serial.println("Button pressed");
		digitalWrite(LED2_PIN, state);
	} else {
		Serial.println("Always low...");
		if (state == HIGH) {
			state = LOW;
			Serial.println("Button released");
			digitalWrite(LED2_PIN, state);
		} else {
			Serial.println("Heartbeat");
		}
	}
}

//
// An example of a setup function.
//
void setup() {

	// We can "start" serial.
	Serial.begin();

	// We can use if(Serial), although this will always return true now (might be different in release mode).
	if (!Serial) return;

	// Set GPIO pin to OUTPUT, this is pin P1.00
	pinMode(GPIO0_PIN, OUTPUT);
	digitalWrite(GPIO0_PIN, false);

	// Set LED pin to OUTPUT, so we can write.
	pinMode(LED1_PIN, OUTPUT);
	digitalWrite(LED1_PIN, true);
	pinMode(LED3_PIN, OUTPUT);
	digitalWrite(LED3_PIN, true);
	pinMode(LED2_PIN, OUTPUT);
	digitalWrite(LED2_PIN, true);

	// Set interrupt handler
	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	//pinMode(BUTTON2_PIN, INPUT);
	if (!attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), button_pressed, CHANGE)) {
		Serial.println("Setting button interrupt failed");
	}

}

//
// A dummy loop function.
//
void loop() {
	// Do nothing, just return, everything is done in interrupt service routine
}
