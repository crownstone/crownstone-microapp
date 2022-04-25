#pragma once

#include <Serial.h>
#include <Wire.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <microapp.h>
#include <stdint.h>

//
// Redefinitions for easy pin access. These do NOT correspond to physical pins on a specific board.
// A maximum of 10 GPIO pins (0-9), 4 button pins (1-4) and 4 LED pins (1-4) are supported.
// However, keep in mind that for a specific board some of these pins may not exist.
// The physical pin mapping for your board is defined in bluenet/source/include/boards/{board}.h
//
const uint8_t GPIO0_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO0;
const uint8_t GPIO1_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO1;
const uint8_t GPIO2_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO2;
const uint8_t GPIO3_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO3;
const uint8_t GPIO4_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO4;
const uint8_t GPIO5_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO5;
const uint8_t GPIO6_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO6;
const uint8_t GPIO7_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO7;
const uint8_t GPIO8_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO8;
const uint8_t GPIO9_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_GPIO9;
const uint8_t BUTTON1_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_BUTTON1;
const uint8_t BUTTON2_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_BUTTON2;
const uint8_t BUTTON3_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_BUTTON3;
const uint8_t BUTTON4_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_BUTTON4;
const uint8_t LED1_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_LED1;
const uint8_t LED2_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_LED2;
const uint8_t LED3_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_LED3;
const uint8_t LED4_PIN = CommandMicroappPin::CS_MICROAPP_COMMAND_PIN_LED4;

//
// You have to implement the setup() function. It can be empty if there is nothing to do at startup.
//
void setup();

//
// The loop function will be performed every so many milliseconds. Make sure your loop returns. If it has a while
// loop that does not return, it will be checked as invalid in the Crownstone code. In the worst case, it can brick
// your device. The same is true if processing is taking too long.
//
void loop();

//
// A delay in ms. Hence 1000 means a delay of one second.
//
void delay(uint32_t delay_ms);

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
int attachInterrupt(uint8_t pin, void (*isr)(void), uint8_t mode);

// Mapping from digital pins to interrupts.
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
