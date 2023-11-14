#include <OneWire.h>
#include <Wire.h>

void (*resetFunc)(void) = 0;

char serial_buffer[SERIAL_BUFFER_SIZE] = {0};
uint16_t serial_buffer_index = 0;

OneWire oneWire;
uint8_t rom[8] = {0};
uint8_t data[9] = {0};
uint32_t previousMillis = 0;

void appendToBuffer(const char* format, ...) {
  va_list args;
  va_start(args, format);
  int result = vsnprintf(serial_buffer + serial_buffer_index, SERIAL_BUFFER_SIZE - serial_buffer_index, format, args);
  va_end(args);

  if (result >= 0 && serial_buffer_index + result < SERIAL_BUFFER_SIZE) {
    serial_buffer_index += result;
  } else {
    serial_buffer_index = 0;
    Serial.println(F("{\"error\": \"full_buffer\"}"));
  }
}

void oneWireCheck() {
  if (!oneWire.checkPresence()) {
    Serial.println(F("{\"error\": \"no_device\"}"));
    Serial.flush();
    digitalWrite(POWER_PIN, LOW);
    delay(5000);
    resetFunc();
  }
}

void oneWirePrint(uint8_t deviceAddress[], int16_t temp) {
  appendToBuffer(",\"0x");

  for (uint8_t i = 0; i < 8; i++) {
    appendToBuffer("%02X", deviceAddress[i]);
  }
  appendToBuffer("\": %i.%u", temp / 16, abs(temp % 16) * 1000 / 16);
}

void oneWireRead() {
  digitalWrite(STATUS_PIN, HIGH);

  appendToBuffer("{\"error\": \"ok\"");

  for (int i = 0; i < NO_CHANNELS; i++) {
    oneWire.setChannel(i);

    // check if sensor on bus
    if (!oneWire.reset()) {
      continue;
    }

    while (oneWire.wireSearch(rom)) {
      oneWire.reset();
      oneWire.select(rom);

      // start conversion
      oneWire.write(0x44, true);

      // wait for conversion
      delay(850);

      // check serial now due to delays
      serialRead();

      oneWire.reset();
      oneWire.select(rom);

      // read Scratchpad
      oneWire.write(0xBE);

      for (uint8_t i = 0; i < sizeof(data); i++) {
        data[i] = oneWire.read();
      }

      // check CRC
      if (!OneWire::crc8(data, 8)) {
        Serial.print(F("{\"error\": \"crc_channel_"));
        Serial.print(i);
        Serial.println(F("\"}"));
        break;
      }

      int16_t raw = (data[1] << 8) | data[0];

      switch (rom[0]) {
        // DS18S20
        case 0x10: {
          raw = raw << 3;

          if (data[7] == 0x10) {
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        } break;

          // DS18B20
        case 0x28: {
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
      oneWirePrint(rom, raw);
    }

    oneWire.reset_search();
  }

  appendToBuffer("}");
  digitalWrite(STATUS_PIN, LOW);

  // send
  Serial.println(serial_buffer);
  serial_buffer_index = 0;
  memset(serial_buffer, 0, SERIAL_BUFFER_SIZE);
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