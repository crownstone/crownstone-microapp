//
// An example to show a few functions being implemented.
//

#include <Arduino.h>

// Show how a counter is incremented
static int counter = 0;

#define GENERATE_FAULT_IN_INTERRUPT_HANDLER 0
#define GENERATE_FAULT_IN_SETUP_FUNCTION    0
#define GENERATE_FAULT_IN_LOOP_FUNCTION     1

void introduceFaultInInterruptHandler() {
#if GENERATE_FAULT_IN_INTERRUPT_HANDLER == 1
	while(true) {};
#endif
	Serial.println("Interrupt triggered");
}

void setup() {
	Serial.begin();
	if (!Serial) return;
	Serial.println("Faulty microapp");

#if GENERATE_FAULT_IN_SETUP_FUNCTION == 1
	while(true) {};
#endif

	pinMode(BUTTON2_PIN, INPUT_PULLUP);
	// Set interrupt handler
	attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), introduceFaultInInterruptHandler, CHANGE);
}

void loop() {
#if GENERATE_FAULT_IN_LOOP_FUNCTION == 1
	while(true) {};
#endif

	// Show counter.
	Serial.println("Loop");
	Serial.println(++counter);
}
