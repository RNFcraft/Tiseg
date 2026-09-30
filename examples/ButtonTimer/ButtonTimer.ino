#include <Tiseg.h>

// Common-anode digit pins.
const uint8_t digitPins[] = {13, 12, 11, 10};

// Segments: A, B, C, D, E, F, G, DP.
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

const uint8_t BUTTON_PIN = 9;

Tiseg<4> display(digitPins, segmentPins);
TisegTimerControl timer(BUTTON_PIN);

// The timer value stays in the sketch and can be used anywhere.
int seconds = 0;

void setup() {
    display.begin();
    timer.begin();

    // Ready/reset state.
    display.printR(0, true); // 0000
}

void loop() {
    display.tick();
    timer.tick();

    // First press: run the editable timer loop.
    if (timer.justStarted()) {
        for (seconds = 0; seconds <= 60; seconds++) {
            // New number becomes visible immediately.
            // While running, do not fill unused digits with zeros.
            display.printR(seconds, false);

            // Stop at 60. Change 60 to any value or change the loop itself.
            if (seconds == 60) {
                timer.pause();
                break;
            }

            // One timer step:
            //   first 500 ms  -> current number is visible
            //   second 500 ms -> display is blank
            // Then the for-loop switches to the next number and printR()
            // lights the display again with that new value.
            //
            // The button remains responsive during both halves.
            if (!timer.waitBlink(1000, display)) {
                break;
            }
        }
    }

    // Second press during counting, or automatic stop at 60.
    if (timer.justPaused()) {
        // Paused/stopped state: display stays on and leading zeros are shown.
        display.printR(seconds, true);
    }

    // Third press: reset.
    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true); // 0000
    }
}
