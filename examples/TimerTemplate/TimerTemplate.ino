#include <Tiseg.h>

// --------------------------------------------------
// CONNECTION
// --------------------------------------------------

// Common-anode digit pins.
const uint8_t digitPins[] = {13, 12, 11, 10};

// Segments: A, B, C, D, E, F, G, DP.
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

// Button: pin 9 -> button -> GND.
const uint8_t BUTTON_PIN = 9;

Tiseg<4> display(digitPins, segmentPins);
TisegTimerControl timer(BUTTON_PIN);

int seconds = 0;

void setup() {
    display.begin();
    timer.begin();

    // Before start and after reset.
    display.printR(0, true); // 0000
}

void loop() {
    display.tick();
    timer.tick();

    // --------------------------------------------------
    // FIRST PRESS: START TIMER
    // --------------------------------------------------
    if (timer.justStarted()) {

        // EDIT THE TIMER HERE.
        // Example: 0 -> 60 seconds.
        for (seconds = 0; seconds <= 60; seconds++) {

            // During counting: no leading zeros.
            // ___0, ___1, ___2, ... __12
            display.printR(seconds, false);

            // Last value of this for-loop.
            if (seconds == 60) {
                timer.pause();
                break;
            }

            // One second:
            //   0-500 ms    -> number is visible
            //   500-1000 ms -> display is blank
            // Then the next for-loop value is shown.
            //
            // The button stays responsive during the whole second.
            if (!timer.waitBlink(1000, display)) {
                break;
            }
        }
    }

    // --------------------------------------------------
    // SECOND PRESS: PAUSE
    // --------------------------------------------------
    if (timer.justPaused()) {
        // On pause/stopping: fill with leading zeros.
        // Example: 0012, 0060.
        display.printR(seconds, true);
    }

    // --------------------------------------------------
    // THIRD PRESS: RESET
    // --------------------------------------------------
    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true); // 0000
    }
}

/*
HOW TO EDIT THE TIMER

1. Count to 120 seconds:

   for (seconds = 0; seconds <= 120; seconds++) {
       ...
   }

   and change:

   if (seconds == 120) {
       timer.pause();
       break;
   }

2. Change the speed:

   timer.waitBlink(500, display);   // 0.5 second per step
   timer.waitBlink(1000, display);  // 1 second per step
   timer.waitBlink(2000, display);  // 2 seconds per step

3. Disable blinking:

   Replace:

   timer.waitBlink(1000, display)

   with:

   timer.wait(1000, display)

BUTTON CYCLE

1st press -> start
2nd press -> pause
3rd press -> reset to 0000
*/
