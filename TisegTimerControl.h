#pragma once
#include <Arduino.h>
#include "TisegButton.h"

/**
 * TisegTimerControl — small helper for the usual one-button timer workflow.
 *
 * It does NOT decide how long the timer runs and does NOT own the timer value.
 * The sketch can use an ordinary for-loop to define the timer range.
 *
 * Button cycle:
 *   READY -> RUNNING -> PAUSED -> READY
 */
class TisegTimerControl {
public:
    enum State {
        READY,
        RUNNING,
        PAUSED
    };

    TisegTimerControl(
        uint8_t buttonPin,
        unsigned long debounceMs = 50,
        bool activeLow = true
    ) : _button(buttonPin, debounceMs, activeLow) {}

    /** Configure the button. Call once from setup(). */
    void begin() {
        _button.begin();
        _state = READY;
        clearEvents();
    }

    /** Poll the button and update the start/pause/reset state machine. */
    void tick() {
        _button.tick();

        if (!_button.wasPressed()) return;

        switch (_state) {
            case READY:
                start();
                break;

            case RUNNING:
                pause();
                break;

            case PAUSED:
                reset();
                break;
        }
    }

    /** Switch to RUNNING. Useful both manually and through the button cycle. */
    void start() {
        if (_state == RUNNING) return;
        _state = RUNNING;
        clearEvents();
        _startedEvent = true;
    }

    /** Switch RUNNING -> PAUSED. */
    void pause() {
        if (_state != RUNNING) return;
        _state = PAUSED;
        clearEvents();
        _pausedEvent = true;
    }

    /** Return to READY. The sketch decides what value should be reset. */
    void reset() {
        _state = READY;
        clearEvents();
        _resetEvent = true;
    }

    /**
     * Wait while keeping a tickable device (for example Tiseg display)
     * refreshed and the button responsive.
     *
     * Designed for simple editable timer loops:
     *
     *   for (int sec = 0; sec <= 60; sec++) {
     *       display.printR(sec);
     *       if (!timer.wait(1000, display)) break;
     *   }
     *
     * Returns false if the timer leaves RUNNING state during the wait.
     */
    template <typename Tickable>
    bool wait(unsigned long durationMs, Tickable& tickable) {
        if (_state != RUNNING) return false;

        unsigned long startedAt = millis();

        while (_state == RUNNING && millis() - startedAt < durationMs) {
            tickable.tick();
            tick();
        }

        return _state == RUNNING;
    }

    /**
     * Timer wait with a blink transition.
     *
     * For the first half of durationMs the current value is visible.
     * For the second half the display is completely blank.
     * On normal completion the display remains blank until the next print(),
     * so the next visible value is already the next number from the for-loop.
     *
     * Example for a one-second timer step:
     *   value 1 -> visible 500 ms -> blank 500 ms -> value 2
     *
     * If the timer is paused during the wait, the display is enabled again
     * before returning false so the paused value can be shown immediately.
     */
    template <typename Display>
    bool waitBlink(unsigned long durationMs, Display& display) {
        if (_state != RUNNING) {
            display.show();
            return false;
        }

        unsigned long startedAt = millis();
        unsigned long blankAt = durationMs / 2UL;
        bool blanked = false;

        display.show();

        while (_state == RUNNING) {
            unsigned long elapsed = millis() - startedAt;
            if (elapsed >= durationMs) break;

            if (!blanked && elapsed >= blankAt) {
                display.hide();
                blanked = true;
            }

            if (!blanked) {
                display.tick();
            }

            tick();
        }

        if (_state != RUNNING) {
            display.show();
            return false;
        }

        // Finish the step blank. The next display.print*() call enables the
        // display only after its new number has been written to the buffer.
        display.hide();
        return true;
    }

    State state() const {
        return _state;
    }

    bool isReady() const {
        return _state == READY;
    }

    bool isRunning() const {
        return _state == RUNNING;
    }

    bool isPaused() const {
        return _state == PAUSED;
    }

    /** One-shot event: true once after a transition to RUNNING. */
    bool justStarted() {
        bool event = _startedEvent;
        _startedEvent = false;
        return event;
    }

    /** One-shot event: true once after a transition to PAUSED. */
    bool justPaused() {
        bool event = _pausedEvent;
        _pausedEvent = false;
        return event;
    }

    /** One-shot event: true once after a transition to READY/reset. */
    bool justReset() {
        bool event = _resetEvent;
        _resetEvent = false;
        return event;
    }

    /** Access the underlying button for advanced/custom behaviour. */
    TisegButton& button() {
        return _button;
    }

private:
    TisegButton _button;
    State _state = READY;

    bool _startedEvent = false;
    bool _pausedEvent = false;
    bool _resetEvent = false;

    void clearEvents() {
        _startedEvent = false;
        _pausedEvent = false;
        _resetEvent = false;
    }
};
