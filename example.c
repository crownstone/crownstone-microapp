#include <Arduino.h>

/*
 * A dummy setup function.
 */
void setup() {
}

static int i = 2;

/*
 * A dummy loop function.
 */
int loop() {
	i++;
	return i;
//
//	return (int)&i;
}
