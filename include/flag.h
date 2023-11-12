void flag() {
  digitalWrite(POWER_PIN, LOW);
  Serial.println(F("{\"error\": \"overcurrent\"}"));
  Serial.flush();
  delay(5000);
  resetFunc();
}

void setupFlg() {
  pinMode(FLG_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLG_PIN), flag, FALLING);

  // enable interrupts
  interrupts();
}
