#pragma once
#include <Arduino.h>
#include "Tiseg.h"

/**
 * TisegTimer — ready-to-use one-button timer built on top of Tiseg.
 *
 * Button cycle:
 *   READY -> RUNNING -> PAUSED -> READY(reset)
 *
 * Display behaviour:
 *   READY:   0000
 *   RUNNING: ___0, ___1, __12 ... (no leading zero fill)
 *   PAUSED:  0001, 0012 ...       (leading zeros enabled)
 *   FINISHED:0060                  (leading zeros enabled)
 */
template <uint8_t DIGITS>
class TisegTimer {
public:
    enum State {
        READY,
        RUNNING,
        PAUSED,
        FINISHED
    };

    /**
     * @param digitPins   DIGITS digit-select pins.
     * @param segmentPins 8 segment pins in order a,b,c,d,e,f,g,dp.
     * @param buttonPin   Button pin. Default TisegButton wiring is pin -> button -> GND.
     * @param maxSeconds  Value at which the timer stops automatically.
     */
    TisegTimer(
        const uint8_t* digitPins,
        const uint8_t* segmentPins,
        uint8_t buttonPin,
        unsigned long maxSeconds = 60UL
    ) :
        _display(digitPins, segmentPins),
        _button(buttonPin),
        _maxSeconds(maxSeconds) {}

    /** Configure display/button and show 0000. Call once from setup(). */
    void begin() {
        _display.begin();
        _button.begin();
        reset();
    }

    /**
     * Update display, button and timer. Call continuously from loop().
     * No delay() and no blocking loops are used.
     */
    void tick() {
        _display.tick();
        _button.tick();

        if (_button.wasPressed()) {
            handleButtonPress();
        }

        if (_state != RUNNING) {
            return;
        }

        unsigned long now = millis();

        while (_state == RUNNING && now - _lastSecondAt >= 1000UL) {
            _lastSecondAt += 1000UL;

            if (_seconds < _maxSeconds) {
                _seconds++;
            }

            if (_seconds >= _maxSeconds) {
                _seconds = _maxSeconds;
                _state = FINISHED;
                // Timer stopped: show leading zeros, for example 0060.
                _display.printR((long)_seconds, true);
            } else {
                // While running, keep unused digits blank.
                _display.printR((long)_seconds, false);
            }
        }
    }

    /** Start from the current value. */
    void start() {
        if (_state == RUNNING || _state == FINISHED) return;

        _state = RUNNING;
        _lastSecondAt = millis();
        // As soon as the timer starts, remove leading zero fill.
        _display.printR((long)_seconds, false);
    }

    /** Pause and show the current value with leading zeros. */
    void pause() {
        if (_state != RUNNING) return;

        _state = PAUSED;
        _display.printR((long)_seconds, true);
    }

    /** Reset to the ready state and show 0000. */
    void reset() {
        _seconds = 0;
        _state = READY;
        _lastSecondAt = millis();
        _display.printR(0, true);
    }

    /** Current timer state. */
    State state() const {
        return _state;
    }

    /** Current timer value in seconds. */
    unsigned long value() const {
        return _seconds;
    }

    /** Maximum timer value. */
    unsigned long maxSeconds() const {
        return _maxSeconds;
    }

    /** Access the underlying display for advanced use. */
    Tiseg<DIGITS>& display() {
        return _display;
    }

    /** Access the underlying button for advanced use. */
    TisegButton& button() {
        return _button;
    }

private:
    Tiseg<DIGITS> _display;
    TisegButton _button;

    unsigned long _maxSeconds;
    unsigned long _seconds = 0;
    unsigned long _lastSecondAt = 0;
    State _state = READY;

    void handleButtonPress() {
        switch (_state) {
            case READY:
                start();
                break;

            case RUNNING:
                pause();
                break;

            case PAUSED:
            case FINISHED:
                reset();
                break;
        }
    }
};
