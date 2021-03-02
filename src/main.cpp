#include <Arduino.h>
#include "cosio.h"

void setup() {
 COSIO::instance()->setup();
}

void loop() {
 COSIO::instance()->loop();
}
