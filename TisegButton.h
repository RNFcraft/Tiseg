#pragma once
#include <Arduino.h>

/**
 * TisegButton — small non-blocking button helper for Arduino.
 *
 * Default wiring uses INPUT_PULLUP:
 *   pin --- button --- GND
 *
 * Call tick() as often as possible from loop(). A callback registered with
 * onPress() is called once for every debounced button press.
 */
class TisegButton {
public:
    typedef void (*Callback)();

    /**
     * @param pin        Arduino pin connected to the button.
     * @param debounceMs Debounce time in milliseconds.
     * @param activeLow  true for INPUT_PULLUP wiring (pressed = LOW).
     */
    TisegButton(uint8_t pin, unsigned long debounceMs = 50, bool activeLow = true)
        : _pin(pin), _debounceMs(debounceMs), _activeLow(activeLow) {}

    /** Configure the button pin. Call once from setup(). */
    void begin() {
        pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT);

        bool pressedNow = readPressed();
        _rawPressed = pressedNow;
        _stablePressed = pressedNow;
        _changedAt = millis();
        _pressEvent = false;
        _releaseEvent = false;
    }

    /**
     * Poll the button and process debounce/callbacks.
     * This method does not use delay() and does not block loop().
     */
    void tick() {
        unsigned long now = millis();
        bool pressedNow = readPressed();

        if (pressedNow != _rawPressed) {
            _rawPressed = pressedNow;
            _changedAt = now;
        }

        if (_stablePressed != _rawPressed && now - _changedAt >= _debounceMs) {
            _stablePressed = _rawPressed;

            if (_stablePressed) {
                _pressEvent = true;
                if (_onPress != nullptr) _onPress();
            } else {
                _releaseEvent = true;
                if (_onRelease != nullptr) _onRelease();
            }
        }
    }

    /** Assign a function that is called once when the button is pressed. */
    void onPress(Callback callback) {
        _onPress = callback;
    }

    /** Assign a function that is called once when the button is released. */
    void onRelease(Callback callback) {
        _onRelease = callback;
    }

    /** Current debounced button state. */
    bool isPressed() const {
        return _stablePressed;
    }

    /**
     * Event-style API for users who do not want callbacks.
     * Returns true once after each press and then clears the event flag.
     */
    bool wasPressed() {
        bool event = _pressEvent;
        _pressEvent = false;
        return event;
    }

    /** Returns true once after each release and clears the event flag. */
    bool wasReleased() {
        bool event = _releaseEvent;
        _releaseEvent = false;
        return event;
    }

private:
    uint8_t _pin;
    unsigned long _debounceMs;
    bool _activeLow;

    bool _rawPressed = false;
    bool _stablePressed = false;
    bool _pressEvent = false;
    bool _releaseEvent = false;
    unsigned long _changedAt = 0;

    Callback _onPress = nullptr;
    Callback _onRelease = nullptr;

    bool readPressed() const {
        int level = digitalRead(_pin);
        return _activeLow ? (level == LOW) : (level == HIGH);
    }
};
