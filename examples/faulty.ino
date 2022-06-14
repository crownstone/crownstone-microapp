//
// An example to show a few functions being implemented.
//

#include <Arduino.h>

// Show how a counter is incremented
static int counter = 0;

#define GENERATE_FAULT_IN_INTERRUPT_HANDLER 0
#define GENERATE_FAULT_IN_SETUP_FUNCTION    0
#define GENERATE_FAULT_IN_LOOP_FUNCTION     1

#define GENERATE_INFINITE_LOOP              0
#define GENERATE_ILLEGAL_INSTRUCTION        0


void generate_fault() {
#if GENERATE_INFINITE_LOOP == 1
	while(true) {};
#endif
#if GENERATE_ILLEGAL_INSTRUCTION == 1
  void (*bad_instruction)(void) = (void (*)())0xE0000000;
  bad_instruction();
#endif
}


void introduceFaultInInterruptHandler() {
#if GENERATE_FAULT_IN_INTERRUPT_HANDLER == 1
	generate_fault();
#endif
	Serial.println("Interrupt triggered");
}

void setup() {
	Serial.begin();
	if (!Serial) return;
	Serial.println("Faulty microapp! Keep tight!");

#if GENERATE_FAULT_IN_SETUP_FUNCTION == 1
	generate_fault();
#endif

	// TODO: the following is now overwritten by setting the interrupt handler
	//pinMode(BUTTON2_PIN, INPUT_PULLUP);
	// Set interrupt handler
	attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), introduceFaultInInterruptHandler, CHANGE);
}

void loop() {
#if GENERATE_FAULT_IN_LOOP_FUNCTION == 1
	generate_fault();
#endif

	// Show counter.
	Serial.println("Loop");
	Serial.println(++counter);
}
