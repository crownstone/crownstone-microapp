#include <BleUtils.h>

bool convertTwoHexCharsToByte(const char* chars, uint8_t* byte) {
	uint8_t val[2] = {0, 0};  // actually two 4-bit values
	for (uint8_t i = 0; i < 2; i++) {
		if (chars[i] >= '0' && chars[i] <= '9') {
			val[i] = chars[i] - '0';
		}
		else if (chars[i] >= 'a' && chars[i] <= 'f') {
			val[i] = chars[i] - 'a' + 10;
		}
		else if (chars[i] >= 'A' && chars[i] <= 'F') {
			val[i] = chars[i] - 'A' + 10;
		}
		else {
			return false;
		}
	}
	// shift most significant 4-bit value 4 bits to the left and add least significant 4-bit value
	*byte = ((val[0] & 0x0F) << 4) | (val[1] & 0x0F);
	return true;
}

void convertByteToTwoHexChars(uint8_t byte, char* res) {
	uint8_t c[2];  // divide into two 4-bit numbers
	c[0] = (byte >> 4) & 0x0F;
	c[1] = byte & 0x0F;
	for (uint8_t i = 0; i < 2; i++) {
		if (c[i] <= 9) {
			*res = c[i] + '0';
		}
		else {
			*res = c[i] + 'A' - 10;
		}
		res++;
	}
}
