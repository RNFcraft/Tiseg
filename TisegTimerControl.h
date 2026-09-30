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
     * This lets a button press immediately stop the for-loop.
     *
     * Note: this intentionally waits inside the current sketch flow, but it
     * keeps both the display and button serviced during that time.
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
