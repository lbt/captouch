#include "application.h"
#include "captouch.h"

// Define which pins are connected with a 1-10M resistor.
// The first pin (pTouch) will be connected to the touch sensor
// and *must* be D0, D1, D2, D3, D4, A0, A1, A3, A4, A5, A6, or A7
// see: http://docs.spark.io/firmware/#interrupts-attachinterrupt

#define pTouch	D2
#define pSense	D3
#define pLED	D4

CapTouch Touch(pTouch, pSense);

void setup() {
    Touch.setup();
}

void loop() {
    CapTouch::Event touchEvent = Touch.getEvent();

    if (touchEvent == CapTouch::TouchEvent) {
		digitalWrite(pLED, HIGH); // LED on
    } else if (touchEvent == CapTouch::ReleaseEvent) {
		digitalWrite(pLED, LOW);  // LED off
    }

    delay(100);
}
