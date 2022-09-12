#include <Arduino.h>
#include <BleUtils.h>

void setup() {
	UUID16Bit uuid16bit_one(0xABCD);
	UUID16Bit uuid16bit_two("ABCD");
	Serial.println(uuid16bit_one.string());
	Serial.println(uuid16bit_two.uuid());

	if (uuid16bit_one == uuid16bit_two) {
		Serial.println("Equal");
	}
	else {
		Serial.println("Not equal");
	}

	const uint8_t uuid_one_bytes[UUID_128BIT_BYTE_LENGTH] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD,
		0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56};
	UUID128Bit uuid128bit_one(uuid_one_bytes, UUID_128BIT_BYTE_LENGTH);
	UUID128Bit uuid128bit_two("12345678-ABCD-1234-5678-ABCDEF123456");
	Serial.println(uuid128bit_one.string());
	Serial.println(uuid128bit_two.bytes(), UUID_128BIT_BYTE_LENGTH);

	if (uuid128bit_one != uuid128bit_two) {
		Serial.println("Not equal");
	}
	else {
		Serial.println("Equal");
	}

	UUID128Bit uuid128bit_three("ABCD");
	Serial.println(uuid128bit_three.string());
}

void loop() {
}

