//
// Microapp for smart outlet.
//
// We check for events from BUTTON2.
// We turn on GPIO0
// An example to show a few functions being implemented.
//

#include <Arduino.h>

// Show how a counter is incremented
static int counter = 100;

volatile byte state = LOW;
volatile byte state2 = LOW;

//
// The blink function will be called on interrupt CHANGE. This means when you press and when you release a button.
// Therefore two state variables are used to only toggle the state every two interrupts.
void blink() {
	Serial.println("Toggle");
	state2 = !state2;
	if (state2 == LOW) {
		state = !state;
		digitalWrite(LED2_PIN, state);
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

	// We can also write integers.
	Serial.println(counter);
	// Set GPIO pin to OUTPUT, so we can write.
	pinMode(GPIO0_PIN, OUTPUT);
	digitalWrite(GPIO0_PIN, true);

	//pinMode(GPIO1_PIN, OUTPUT);
	//digitalWrite(GPIO1_PIN, false);

	// Set LED pin to OUTPUT, so we can write.
	pinMode(LED1_PIN, OUTPUT);
	digitalWrite(LED1_PIN, true);
	pinMode(LED3_PIN, OUTPUT);
	digitalWrite(LED3_PIN, true);
	pinMode(LED2_PIN, OUTPUT);
	digitalWrite(LED2_PIN, true);

	// Set interrupt handler
	//pinMode(BUTTON2_PIN, INPUT_PULLUP);
	pinMode(BUTTON2_PIN, INPUT);
	if (!attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), blink, CHANGE)) {
		Serial.println("Setting button interrupt failed");
	}

}

//
// A dummy loop function.
//
void loop() {
	// We are able to use static variables.
	counter++;

	if (counter % 5 == 0) {
		if (state == LOW) {
			Serial.println("State down");
		} else {
			Serial.println("State up");
		}

	}

	if (counter % 10 == 0) {
		Serial.println("Delay");
		// This is in ms
		delay(10000);

	}
	// Show counter.
	Serial.println(counter);

	//Serial.write("button ");
	//int btn = digitalRead(BUTTON2_PIN);
	//Serial.println(btn);
}
