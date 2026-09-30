#include <Tiseg.h>

// Common-anode digit pins.
const uint8_t digitPins[] = {13, 12, 11, 10};

// Segments: A, B, C, D, E, F, G, DP.
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

// 4-digit timer, button on pin 9, count from 0 to 60 seconds.
TisegTimer<4> timer(digitPins, segmentPins, 9, 60);

void setup() {
    timer.begin(); // Shows 0000.
}

void loop() {
    timer.tick();
}

/*
Button behaviour:
  1st press -> start:  ___0, ___1, ___2 ... __59
  2nd press -> pause: 0001, 0012, 0059 ...
  3rd press -> reset: 0000

If the timer reaches 60 by itself, it stops at 0060.
The next press resets it to 0000.
*/
