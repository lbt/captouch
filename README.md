CapTouch is library to provide simple touch sensor for the Spark.

A port of John van Saders' code to a library and a C++/Class based solution.

See: http://jvs.me/touch-sensing-on-the-spark-core/

Usage:

```c++

#include "captouch.h"

// Define which pins are connected with a 1-10M resistor.
// The first pin will be connected to the touch sensor
// and *must* be D0, D1, D2, D3, D4 A0, A1, A3, A4, A5, A6, A7
// see: http://docs.spark.io/firmware/#interrupts-attachinterrupt
CapTouch Touch(D4, D5);

setup() {
    Touch.setup();
}

loop() {

    CapTouch::Event touchEvent = Touch.getEvent();

    if (touchEvent == CapTouch::TouchEvent) {
		digitalWrite(D7, HIGH); // LED on
    } else if (touchEvent == CapTouch::ReleaseEvent) {
		digitalWrite(D7, LOW);  // LED off
    }

    delay(100);
}

```
