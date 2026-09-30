#pragma once
#include <Arduino.h>
#include "TisegButton.h"

/**
 * Tiseg — lightweight driver for directly connected multiplexed
 * common-anode 7-segment displays.
 *
 * Electrical polarity for a direct common-anode connection:
 *   digit/common anode: HIGH = enabled, LOW = disabled
 *   segment cathode:    LOW  = lit,     HIGH = off
 *
 * Tiseg itself only handles number output and multiplexing.
 * TisegButton and TisegTimerControl are available through this same header,
 * so sketches only need #include <Tiseg.h>.
 *
 * DIGITS is the number of display digits, for example Tiseg<4>.
 */
template <uint8_t DIGITS>
class Tiseg {
public:
    Tiseg(const uint8_t* digitPins, const uint8_t* segmentPins)
        : _digitPins(digitPins), _segmentPins(segmentPins) {}

    void begin() {
        // Direct common-anode connection: LOW keeps a digit disabled.
        for (uint8_t i = 0; i < DIGITS; i++) {
            pinMode(_digitPins[i], OUTPUT);
            digitalWrite(_digitPins[i], DIGIT_OFF);
        }

        // Segment cathodes are active LOW, so HIGH means off.
        for (uint8_t i = 0; i < 8; i++) {
            pinMode(_segmentPins[i], OUTPUT);
            digitalWrite(_segmentPins[i], SEGMENT_OFF);
        }

        _enabled = true;
        clear();
    }

    void tick() {
        if (!_enabled) return;
        multiplex();
    }

    void print(long num, bool fillZeros = false) {
        printR(num, fillZeros);
    }

    void printR(long num, bool fillZeros = false) {
        render(num, false, fillZeros);
    }

    void printL(long num, bool fillZeros = false) {
        render(num, true, fillZeros);
    }

    void clear() {
        for (uint8_t i = 0; i < DIGITS; i++) {
            _screen[i] = 0;
        }
    }

    /**
     * Enable or blank the physical display without changing the buffer.
     * Calling print()/printR()/printL() enables the display again.
     */
    void setEnabled(bool enabled) {
        if (_enabled == enabled) return;

        _enabled = enabled;

        if (!_enabled) {
            disableDigits();

            for (uint8_t s = 0; s < 8; s++) {
                digitalWrite(_segmentPins[s], SEGMENT_OFF);
            }
        } else {
            // Let the next tick refresh immediately.
            _stepAt = 0;
        }
    }

    void show() {
        setEnabled(true);
    }

    void hide() {
        setEnabled(false);
    }

    bool isEnabled() const {
        return _enabled;
    }

private:
    const uint8_t* _digitPins;
    const uint8_t* _segmentPins;

    uint8_t _screen[DIGITS] = {0};
    uint8_t _digit = 0;
    unsigned long _stepAt = 0;
    bool _enabled = true;

    static const uint8_t DIGIT_ON = HIGH;
    static const uint8_t DIGIT_OFF = LOW;
    static const uint8_t SEGMENT_ON = LOW;
    static const uint8_t SEGMENT_OFF = HIGH;

    static const uint8_t SEG7[10];
    static const uint8_t MINUS = 0x40;

    void disableDigits() {
        for (uint8_t i = 0; i < DIGITS; i++) {
            digitalWrite(_digitPins[i], DIGIT_OFF);
        }
    }

    void multiplex() {
        unsigned long now = millis();
        if (now - _stepAt < 2) return;
        _stepAt = now;

        // Disable every common anode before changing segment lines.
        // This prevents ghosting between digits.
        disableDigits();

        uint8_t code = _screen[_digit];

        // Prepare all segment cathodes while all digits are disabled.
        for (uint8_t s = 0; s < 8; s++) {
            digitalWrite(
                _segmentPins[s],
                (code & (1 << s)) ? SEGMENT_ON : SEGMENT_OFF
            );
        }

        // Enable only the selected common-anode digit.
        digitalWrite(_digitPins[_digit], DIGIT_ON);

        if (++_digit >= DIGITS) {
            _digit = 0;
        }
    }

    void render(long num, bool alignLeft, bool fillZeros) {
        // A new value should become visible even if the display was blanked.
        setEnabled(true);
        clear();

        bool negative = (num < 0);
        unsigned long value = negative
            ? (unsigned long)(-(num + 1L)) + 1UL
            : (unsigned long)num;

        uint8_t available = DIGITS;
        if (negative && available > 0) {
            available--;
        }

        if (available == 0) {
            if (negative && DIGITS > 0) {
                _screen[0] = MINUS;
            }
            return;
        }

        uint8_t digits[DIGITS];
        uint8_t count = 0;

        do {
            digits[count++] = value % 10;
            value /= 10;
        } while (value > 0 && count < available);

        if (fillZeros) {
            for (uint8_t i = 0; i < DIGITS; i++) {
                _screen[i] = SEG7[0];
            }
        }

        if (alignLeft) {
            uint8_t pos = 0;

            if (negative && pos < DIGITS) {
                _screen[pos++] = MINUS;
            }

            for (uint8_t i = 0; i < count && pos < DIGITS; i++) {
                _screen[pos++] = SEG7[digits[count - 1 - i]];
            }
        } else {
            int16_t pos = (int16_t)DIGITS - 1;

            for (uint8_t i = 0; i < count && pos >= 0; i++) {
                _screen[pos--] = SEG7[digits[i]];
            }

            if (negative && pos >= 0) {
                _screen[pos] = MINUS;
            }
        }
    }
};

template <uint8_t DIGITS>
const uint8_t Tiseg<DIGITS>::SEG7[10] = {
    0x3F,
    0x06,
    0x5B,
    0x4F,
    0x66,
    0x6D,
    0x7D,
    0x07,
    0x7F,
    0x6F
};

#include "TisegTimerControl.h"
