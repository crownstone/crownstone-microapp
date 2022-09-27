#include <Arduino.h>
#include <String.h>

void setup() {
	// const char* (string literal)
	Serial.println("One");

	// String
	Serial.println(String("Two"));

	// int
	Serial.println(3);

	// float
	Serial.println(4.0f);

	// double
	Serial.println(5.0);

	// unsigned int
	Serial.println((unsigned int) 6);

	// short
	Serial.println((short) 0x1234);

	// array
	uint8_t eight[2] = {8, 8};
	Serial.println(eight, 2);
}

void loop() {
	// long string
	Serial.println("Thisstringisprobablytoolongtoprintsowillbetruncated");
}
