#include <Arduino.h>
#include <String.h>

void setup() {
	Serial.println("One"); // const char* (string literal)
	Serial.println(String("Two")); // String
	Serial.println(3); // int
	Serial.println(4.0f); // float
	Serial.println(5.0); // double
	Serial.println((unsigned int) 6); // unsigned int
	Serial.println((short) 0x1234); // short
	uint8_t eight[2] = {8, 8};
	Serial.println(eight, 2); // array
}

void loop() {
	Serial.println("Thisstringisprobalytoolongtoprintsowillbetruncated");
}
