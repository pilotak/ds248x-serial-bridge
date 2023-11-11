void flag() {
  Serial.println(F("{\"error\": \"overcurrent\"}"));
  digitalWrite(POWER_PIN, LOW);
  delay(5000);
  resetFunc();
}

void setupFlg() {
  pinMode(FLG_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLG_PIN), flag, FALLING);

  // enable interrupts
  interrupts();
}
