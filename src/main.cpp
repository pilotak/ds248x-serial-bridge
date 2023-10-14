#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h>

OneWire oneWire;
uint8_t rom[8] = {0};
uint8_t data[9] = {0};

void (*resetFunc)(void) = 0;

void print(uint8_t deviceAddress[], float temp) {
  Serial.print(F("\"0x"));

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

void checkPresence() {
  if (!oneWire.checkPresence()) {
    Serial.println(F("{\"error\": \"no_device\"}"));
    delay(5000);
    resetFunc();
  }
}

void setup() {
  Serial.begin(115200);

  checkPresence();

  oneWire.deviceReset();
}

void loop() {
  checkPresence();
  Serial.print("{");
  bool first = true;

  for (int i = 0; i < NO_CHANNELS; i++) {
    oneWire.setChannel(i);

    if (!oneWire.reset()) {
      continue;
    }

    oneWire.skip();

    // start conversion
    oneWire.write(0x44, true);

    // wait for conversion
    delay(750);

    oneWire.reset();

    while (oneWire.wireSearch(rom)) {
      // Read Scratchpad
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

          char cfg = (data[4] & 0x60);  // default is 12 bit resolution

          if (cfg == 0x00) {  // 9 bit resolution
            raw &= ~7;

          } else if (cfg == 0x20) {  // 10 bit res
            raw &= ~3;

          } else if (cfg == 0x40) {  // 11 bit res
            raw &= ~1;
          }
        }

        default:
          break;
      }

      // print data
      if (!first) {
        Serial.print(F(", "));
      }

      first = false;

      print(rom, ((float)raw) / 16);
    }

    oneWire.reset_search();
  }
  Serial.println(F("}"));
  delay(20000);
}
