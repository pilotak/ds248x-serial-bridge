#include <OneWire.h>
#include <Wire.h>

void (*resetFunc)(void) = 0;

OneWire oneWire;
uint8_t rom[8] = {0};
uint8_t data[9] = {0};
uint32_t previousMillis = 0;

void oneWireCheck() {
  if (!oneWire.checkPresence()) {
    Serial.println(F("{\"error\": \"no_device\"}"));
    Serial.flush();
    digitalWrite(POWER_PIN, LOW);
    delay(5000);
    resetFunc();
  }
}

void oneWirePrint(uint8_t deviceAddress[], float temp) {
  Serial.print(F(", \"0x"));

  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address
    if (deviceAddress[i] < 16) {
      Serial.print(F("0"));
    }
    Serial.print(deviceAddress[i], HEX);
  }

  Serial.print(F("\": "));
  Serial.print(temp, 2);
}

void oneWireRead() {
  digitalWrite(STATUS_PIN, HIGH);

  Serial.print(F("{\"error\": \"ok\""));

  for (int i = 0; i < NO_CHANNELS; i++) {
    oneWire.setChannel(i);

    // check if sensor on bus
    if (!oneWire.reset()) {
      continue;
    }

    // check now due to delays
    serialRead();

    // write to all sensors
    oneWire.skip();

    // start conversion
    oneWire.write(0x44, true);

    // wait for conversion
    delay(750);

    oneWire.reset();

    while (oneWire.wireSearch(rom)) {
      // read Scratchpad
      oneWire.write(0xBE);

      for (uint8_t i = 0; i < 9; i++) {
        data[i] = oneWire.read();
      }

      // check CRC
      if (!OneWire::crc8(data, 8)) {
        continue;
      }

      int16_t raw = (data[1] << 8) | data[0];

      switch (rom[0]) {
        case 0x10: {  // DS18S20
          raw = raw << 3;

          if (data[7] == 0x10) {
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        } break;

        case 0x28: {  // DS18B20

          uint8_t cfg = (data[4] & 0x60);  // default is 12 bit resolution

          if (cfg == 0x00) {  // 9 bit resolution
            raw &= ~7;

          } else if (cfg == 0x20) {  // 10 bit res
            raw &= ~3;

          } else if (cfg == 0x40) {  // 11 bit res
            raw &= ~1;
          }
        } break;

        default:
          // skip unknown devices
          continue;
      }

      // print data
      oneWirePrint(rom, ((float)raw) / 16);
    }

    oneWire.reset_search();
  }
  Serial.println(F("}"));
  digitalWrite(STATUS_PIN, LOW);
}

void oneWireSetup() {
  pinMode(POWER_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);

  digitalWrite(POWER_PIN, HIGH);

  // blink for start
  for (uint8_t i = 0; i < 2; i++) {
    digitalWrite(STATUS_PIN, HIGH);
    delay(300);
    digitalWrite(STATUS_PIN, LOW);
    delay(300);
  }

  oneWireCheck();
  oneWire.deviceReset();

  oneWireRead();
}

void oneWireLoop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 20000) {  // 20 sec interval
    previousMillis = currentMillis;

    oneWireCheck();
    oneWireRead();
  }
}