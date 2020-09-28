#pragma once

#include <stdint.h>

/**
 * You have to implement the setup() function. It can be empty if there is nothing to do at startup.
 */
void setup();

/**
 * The loop function will be performed every so many milliseconds. Make sure your loop returns. If it has a while 
 * loop that does not return, it will be checked as invalid in the Crownstone code. In the worst case, it can brick
 * your device. The same is true if processing is taking too long.
 */
int loop();

/**
 * Write to serial. For now this becomes logs in the Crownstone firmware. That is not so useful to the microapp
 * person though. To send it through to UART for a USB dongle is quite limited, for normal Crownstones it is almost
 * useless. It would be fun to write over Bluetooth RFCOMM.
 */
void write(char *message, int length);

/**
 * A bunch of functions that are mainly useful in either development mode, on a Crownstone that is embedded 
 * electronically, etc., not so much for built-in Crownstones.
 */
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogReference(uint8_t mode);
void analogWrite(uint8_t pin, int val);
