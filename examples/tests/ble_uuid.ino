#include <Arduino.h>
#include <BleUuid.h>

const uint8_t uuid_bytes_16bit[UUID_16BIT_BYTE_LENGTH] = {0xCD, 0xAB};
const uint8_t uuid_bytes_128bit[UUID_128BIT_BYTE_LENGTH] = {0x56, 0x34, 0x12, 0xEF, 0xCD, 0xAB,
		0x78, 0x56, 0x34, 0x12, 0xCD, 0xAB, 0x78, 0x56, 0x34, 0x12};

void setup() {

	Uuid uuid_1(uuid_bytes_16bit, UUID_16BIT_BYTE_LENGTH);
	Serial.println(uuid_1.length());
	Serial.println(uuid_1.string());
	Serial.println(uuid_1.fullString());
	Serial.println(uuid_1.bytes(), UUID_16BIT_BYTE_LENGTH);
	Serial.println(uuid_1.fullBytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println("-----");

	delay(2000);


	Uuid uuid_2("ABCD");
	Serial.println(uuid_2.length());
	Serial.println(uuid_2.string());
	Serial.println(uuid_2.fullString());
	Serial.println(uuid_2.bytes(), UUID_16BIT_BYTE_LENGTH);
	Serial.println(uuid_2.fullBytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println("-----");

	delay(2000);

	Uuid uuid_3(uuid_bytes_128bit, UUID_128BIT_BYTE_LENGTH);
	Serial.println(uuid_3.length());
	Serial.println(uuid_3.string());
	Serial.println(uuid_3.fullString());
	Serial.println(uuid_3.bytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println(uuid_3.fullBytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println("-----");

	delay(2000);

	Uuid uuid_4("12345678-ABCD-1234-5678-ABCDEF123456");
	Serial.println(uuid_4.length());
	Serial.println(uuid_4.string());
	Serial.println(uuid_4.bytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println(uuid_4.fullBytes(), UUID_128BIT_BYTE_LENGTH);
	Serial.println("-----");

	delay(2000);

	Uuid uuid_5(0x1234, 1);
	Serial.println(uuid_5.length());
	Serial.println(uuid_5.fullString());
	Serial.println(uuid_5.uuid16());
	Serial.println("-----");

	delay(2000);

	if (uuid_1 == uuid_2) {
		Serial.println("1 and 2 are equal");
	}
	else {
		Serial.println("1 and 2 are not equal");
	}
	if (uuid_2 != uuid_3) {
		Serial.println("2 and 3 are not equal");
	}
	else {
		Serial.println("2 and 3 are equal");
	}

	// Contains invalid characters
	Uuid uuid_6("INVALID8-ABCD-1234-5678-ABCDEF123456");
	if (uuid_6) {
		Serial.println("6 is valid");
	}
	else {
		Serial.println("6 is invalid");
	}
	// Wrong length
	Uuid uuid_7("1234567-ABCD-1234-5678-ABCDEF123456");
	if (uuid_7) {
		Serial.println("7 is valid");
	}
	else {
		Serial.println("7 is invalid");
	}
	Serial.println("-----");

}

void loop() {
}

