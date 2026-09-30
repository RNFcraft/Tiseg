#pragma once
#include <Arduino.h>

/**
 * Tiseg — lightweight driver for multiplexed common-anode 7-segment displays.
 *
 * The library is responsible only for displaying numbers. It does not contain
 * timer, button, delay, pause, or application logic.
 *
 * DIGITS is the number of display digits, for example Tiseg<4>.
 */
template <uint8_t DIGITS>
class Tiseg {
public:
    /**
     * @param digitPins   Digit-select pins, DIGITS items.
     * @param segmentPins Segment pins, 8 items in order: a,b,c,d,e,f,g,dp.
     */
    Tiseg(const uint8_t* digitPins, const uint8_t* segmentPins)
        : _digitPins(digitPins), _segmentPins(segmentPins) {}

    /**
     * Configure display pins. Call once from setup().
     */
    void begin() {
        for (uint8_t i = 0; i < DIGITS; i++) {
            pinMode(_digitPins[i], OUTPUT);
            digitalWrite(_digitPins[i], HIGH);
        }

        for (uint8_t i = 0; i < 8; i++) {
            pinMode(_segmentPins[i], OUTPUT);
            digitalWrite(_segmentPins[i], HIGH);
        }

        clear();
    }

    /**
     * Refresh dynamic indication. Call as often as possible from loop().
     */
    void tick() {
        multiplex();
    }

    /**
     * Default number output: right aligned.
     *
     * print(1)       -> ___1
     * print(1, true) -> 0001
     */
    void print(long num, bool fillZeros = false) {
        printR(num, fillZeros);
    }

    /**
     * Right-aligned number output.
     *
     * printR(12)       -> __12
     * printR(12, true) -> 0012
     */
    void printR(long num, bool fillZeros = false) {
        render(num, false, fillZeros);
    }

    /**
     * Left-aligned number output.
     *
     * printL(12)       -> 12__
     * printL(12, true) -> 1200
     */
    void printL(long num, bool fillZeros = false) {
        render(num, true, fillZeros);
    }

    /**
     * Clear the display buffer.
     */
    void clear() {
        for (uint8_t i = 0; i < DIGITS; i++) {
            _screen[i] = 0;
        }
    }

private:
    const uint8_t* _digitPins;
    const uint8_t* _segmentPins;

    uint8_t _screen[DIGITS] = {0};
    uint8_t _digit = 0;
    unsigned long _stepAt = 0;

    // Segment codes: bits a,b,c,d,e,f,g,dp.
    static const uint8_t SEG7[10];
    static const uint8_t MINUS = 0x40;

    /**
     * Refresh one digit. Common anode: digit LOW, lit segment LOW.
     */
    void multiplex() {
        unsigned long now = millis();
        if (now - _stepAt < 2) return;
        _stepAt = now;

        // Disable all digits before changing segment lines.
        for (uint8_t i = 0; i < DIGITS; i++) {
            digitalWrite(_digitPins[i], HIGH);
        }

        uint8_t code = _screen[_digit];

        for (uint8_t s = 0; s < 8; s++) {
            digitalWrite(_segmentPins[s], (code & (1 << s)) ? LOW : HIGH);
        }

        digitalWrite(_digitPins[_digit], LOW);

        if (++_digit >= DIGITS) {
            _digit = 0;
        }
    }

    /**
     * Convert a signed integer to segment codes in the screen buffer.
     * If the value is wider than the display, the most significant excess
     * digits are discarded.
     */
    void render(long num, bool alignLeft, bool fillZeros) {
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
    0x3F, // 0: a b c d e f
    0x06, // 1: b c
    0x5B, // 2: a b d e g
    0x4F, // 3: a b c d g
    0x66, // 4: b c f g
    0x6D, // 5: a c d f g
    0x7D, // 6: a c d e f g
    0x07, // 7: a b c
    0x7F, // 8: a b c d e f g
    0x6F  // 9: a b c d f g
};
