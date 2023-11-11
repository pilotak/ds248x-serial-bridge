#include <Arduino.h>

void serialRead();

#include "1wire.h"
#include "flag.h"
#include "serial.h"

void setup() {
  Serial.begin(115200);

  setupFlg();
  oneWireSetup();
}

void loop() {
  serialRead();
  oneWireLoop();
}
