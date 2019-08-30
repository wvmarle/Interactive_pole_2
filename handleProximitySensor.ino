void initProximitySensor() {
  pinMode(PROXIMITY_SENSOR_PIN, INPUT);
}

void handleProximitySensor() {
  static uint32_t proximityDetectedTime;
  if (digitalRead(PROXIMITY_SENSOR_PIN)) {                  // Pin high: movement nearby detected.
    proximityDetectedTime = millis();                       // Record when it last happened.
    proximityDetected = true;                               // Makes LEDs go to "active" mode.
  }
  else if (musicPlaying) {                                  // If music playing, use MUSIC_PLAYING_TIMEOUT before falling back to "idle".
    if (millis() - proximityDetectedTime > MUSIC_PLAYING_TIMEOUT) {
      proximityDetected = false;                            // Makes LEDs go to "idle" mode.
    }
  }
  else if (millis() - proximityDetectedTime > PROXIMITY_TIMEOUT) {
    proximityDetected = false;                              // Otherwise, go to "idle" after the regular PROXIMITY_TIMEOUT.
  }
}
