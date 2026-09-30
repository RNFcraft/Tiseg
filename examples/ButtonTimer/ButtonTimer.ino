#include <Tiseg.h>

// Common-anode digit pins.
const uint8_t digitPins[] = {13, 12, 11, 10};

// Segments: A, B, C, D, E, F, G, DP.
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

const uint8_t BUTTON_PIN = 9;
const unsigned long MAX_SECONDS = 60;

Tiseg<4> display(digitPins, segmentPins);
TisegTimerControl timer(BUTTON_PIN);

// Timer logic stays in the sketch and can be freely edited.
unsigned long seconds = 0;
unsigned long lastSecondAt = 0;

void setup() {
    display.begin();
    timer.begin();

    // Ready/reset state: always show 0000.
    display.printR(0, true);
}

void loop() {
    display.tick();
    timer.tick();

    // 1st button press: start.
    if (timer.justStarted()) {
        lastSecondAt = millis();

        // Running state: no leading zeros.
        display.printR((long)seconds, false);
    }

    // 2nd button press: pause.
    if (timer.justPaused()) {
        // Paused state: show leading zeros.
        display.printR((long)seconds, true);
    }

    // 3rd button press: reset.
    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true); // 0000
    }

    // Everything below is ordinary editable timer logic.
    if (timer.isRunning()) {
        unsigned long now = millis();

        while (timer.isRunning() && now - lastSecondAt >= 1000UL) {
            lastSecondAt += 1000UL;
            seconds++;

            // While running: ___1, __12, ...
            display.printR((long)seconds, false);

            // Stop at 60. Change/remove this block for another behaviour.
            if (seconds >= MAX_SECONDS) {
                seconds = MAX_SECONDS;
                timer.pause();

                // Stopped/paused: 0060.
                display.printR((long)seconds, true);
            }
        }
    }
}
