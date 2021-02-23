#pragma once

#include <Serial.h>
#include <Wire.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <microapp.h>
#include <stdint.h>

//
// You have to implement the setup() function. It can be empty if there is nothing to do at startup.
//
void setup();

//
// The loop function will be performed every so many milliseconds. Make sure your loop returns. If it has a while 
// loop that does not return, it will be checked as invalid in the Crownstone code. In the worst case, it can brick
// your device. The same is true if processing is taking too long.
//
int loop();

//
// A delay in ms. Hence 1000 means a delay of one second.
//
void delay(uint16_t delay_ms);

//
// A bunch of functions that are mainly useful in either development mode, on a Crownstone that is embedded 
// electronically, etc., not so much for built-in Crownstones. It can also be the case that these functions are
// made dummies to prevent mistakes. For example, setting the relay as an INPUT rather than an OUTPUT.
//
void init();
void initVariant();
void pinMode(uint8_t pin, uint8_t mode);

//
// Write to particular pins. These are virtual pins and are either mapped to physical pins or drivers.
//
// @param pin 1 turn on/off the relay
//
void digitalWrite(uint8_t pin, uint8_t val);


int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogReference(uint8_t mode);

//
// Write to particular pins. These are virtual pins and are either mapped to physical pins or drivers.
//
// @param pin 1 sets the dimmer to a particular value
//
void analogWrite(uint8_t pin, int val);

//
// Attach an interrupt to a particular pin.
//
// @param pin      Set interrupt on particular pin
// @param isr      The interrupt service routine to call
// @param mode     The options are LOW, CHANGE, RISING, FALLING, and HIGH.
//
void attachInterrupt(uint8_t pin, void (*isr)(void), uint8_t mode);

//
// Mapping from digital pins to interrupts. This will be just the same for Crownstone hardware. Any mapping will be
// done in bluenet.
//
uint8_t digitalPinToInterrupt(uint8_t pin);

typedef bool boolean;
typedef uint8_t byte;

// Return highest byte (8 bits) or second-lowest byte (for larger data types)
byte highByte(short val);

// Return lowest byte (8 bits)
byte lowByte(short val);

// Combine two bytes into a short
short word(byte highByte, byte lowByte);

#ifdef __cplusplus
}
#endif
