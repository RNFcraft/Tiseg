#include <Tiseg.h>

const uint8_t digitPins[]   = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};
const uint8_t BUTTON_PIN = 9;

Tiseg<4> display(digitPins, segmentPins);
TisegButton button(BUTTON_PIN);

enum TimerState {
    TIMER_READY,
    TIMER_RUNNING,
    TIMER_PAUSED
};

TimerState timerState = TIMER_READY;
unsigned long seconds = 0;
unsigned long lastSecondAt = 0;

void handleTimerButton() {
    switch (timerState) {
        case TIMER_READY:
            // 1st press: start counting.
            timerState = TIMER_RUNNING;
            lastSecondAt = millis();
            break;

        case TIMER_RUNNING:
            // 2nd press: pause and keep the current value on screen.
            timerState = TIMER_PAUSED;
            break;

        case TIMER_PAUSED:
            // 3rd press: reset to zero and return to the ready state.
            timerState = TIMER_READY;
            seconds = 0;
            display.printR(seconds, true);
            break;
    }
}

void setup() {
    display.begin();
    button.begin();

    button.onPress(handleTimerButton);

    display.printR(0, true); // 0000
}

void loop() {
    display.tick();
    button.tick();

    if (timerState == TIMER_RUNNING) {
        unsigned long now = millis();

        while (now - lastSecondAt >= 1000UL) {
            lastSecondAt += 1000UL;
            seconds++;
            display.printR((long)seconds, true);
        }
    }
}
