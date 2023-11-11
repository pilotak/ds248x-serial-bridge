#define BUFFER_SIZE 20
#define READ_TIMEOUT 100  // ms

char buffer[BUFFER_SIZE] = {0};
uint8_t buffer_index = 0;
uint32_t last_data_timeout = 0;

void serialParse() {
  if (strstr(buffer, "restart") != NULL) {
    digitalWrite(POWER_PIN, LOW);
    Serial.println(F("{\"restart\": \"ok\"}"));
    delay(1000);
    resetFunc();
  }

  buffer_index = 0;
  memset(buffer, 0, sizeof(buffer));
}

void serialRead() {
  uint32_t m = millis();

  if (buffer_index > 0 && m - last_data_timeout > READ_TIMEOUT) {
    serialParse();
  }

  if (!Serial.available()) {
    return;
  }

  while (Serial.available()) {
    buffer[buffer_index++] = Serial.read();
    last_data_timeout = millis();

    if (buffer_index == BUFFER_SIZE) {  // prevent overflow
      Serial.flush();
      buffer_index = 0;
    }
  }
}
