#pragma once

/**
 * You have to implement the setup() function. It can be empty if there is nothing to do at startup.
 */
void setup();

/**
 * The loop function will be performed every so many milliseconds. Make sure your loop returns. If it has a while 
 * loop that does not return, it will be checked as invalid in the Crownstone code. In the worst case, it can brick
 * your device. The same is true if processing is taking too long.
 */
void loop();
