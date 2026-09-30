#include <Tiseg.h>

// Common-anode digit pins for a 4-digit display.
const uint8_t digitPins[] = {13, 12, 11, 10};

// Segment pins in order: a, b, c, d, e, f, g, dp.
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);

void setup() {
    display.begin();

    // Right aligned, blank unused digits: ___1
    display.printR(1);

    // Other examples:
    // display.printR(1, true);   // 0001
    // display.printL(12);        // 12__
    // display.printL(12, true);  // 1200
    // display.print(345);        // _345 (same as printR)
}

void loop() {
    // Required for dynamic indication.
    display.tick();
}
