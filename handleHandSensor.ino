#include <VL53L0X.h>                                        // https://github.com/ScruffR/VL53L0X
VL53L0X handSensor;                                         // The constructor.

void initHandSensor() {
  Serial.println(F("Starting up hand sensor."));
  handSensor.init();                                        // Initialise the sensor.
  Serial.println(F("Init done."));
  handSensor.setTimeout(0);                                 // Sets a timeout period in milliseconds after which read operations will abort if the sensor is not ready. A value of 0 disables the timeout.
  Serial.println(F("Timeout set."));
  handSensor.startContinuous();                             // Take measurements continuously, as fast as possible.
  Serial.println(F("Readings started."));
  pinMode(2, INPUT_PULLUP);                                 // Make sure we don't leave it floating.
  attachInterrupt(digitalPinToInterrupt(2), sensorInterrupt, FALLING); // When measurement is done, an interrupt is fired (active low).
}

volatile bool haveReading;
void sensorInterrupt() {
  haveReading = true;
}

void handleHandSensor() {
  if (haveReading) {                                        // We got an interrupt, a new reading is complete.
    haveReading = false;
    getReading();                                           // Read the sensor's result. This resets the interrupt state.
    analyseMotion(motionState);
  }
}

void analyseMotion(uint8_t &state) {

  // Motion up/down detection.
  uint8_t nUp = 0;
  uint8_t nDown = 0;
  uint16_t totalUp = 0;
  uint16_t totalDown = 0;
  state = 0;
  for (uint8_t i = 0; i < 10; i++) {
    if (readings[i] > MIN_DISTANCE && readings[i] < MAX_DISTANCE) { // Only use measurment if hand was detected.
      if (readings[i] > readings[i + 1]) {                  // Higher reading - moved up.
        nUp++;
        totalUp += readings[i] - readings[i + 1];
      }
      else {                                                // Lower reading - moved down.
        nDown++;
        totalDown += readings[i + 1] - readings[i];
      }
    }
  }

  // For a motion, require 8 or more of the 10 readings to be in either direction, and the
  // total movement in that direction to be >30 mm. This to filter out noise.
  bitWrite(state, MOTION_DOWN, (nDown > 7 && totalDown > 30));
  bitWrite(state, MOTION_UP, (nUp > 7 && totalUp > 30));

  // Hand presented/removed.
  bool handPresented = true;
  bool handRemoved = true;

  // Check whether a hand is present or not during the latest three readings.
  for (uint8_t i = 0; i < 3; i++) {
    if (readings[i] < MAX_DISTANCE) {                       // Hand present.
      handRemoved = false;
    }
    if (readings[i] > MAX_DISTANCE) {                       // No hand present.
      handPresented = false;
    }
  }

  // Check whether the opposite is true for the previous three readings.
  for (uint8_t i = 3; i < 6; i++) {
    if (handRemoved) {
      if (readings[i] > MAX_DISTANCE) {                     // It wasn't there earlier; not removed now.
        handRemoved = false;
      }
    }
    if (handPresented) {
      if (readings[i] < MAX_DISTANCE) {                     // It was there already; not presented now.
        handPresented = false;
      }
    }
  }
  bitWrite(state, HAND_PRESENTED, handPresented);
  bitWrite(state, HAND_REMOVED, handRemoved);

  // Check for swipe: if hand removed, see if within the range of measurements there's a series of three measurements
  // that show hand not present.
  bool swipe = false;
  uint8_t noHandMeasurements;
  if (handRemoved) {
    for (uint8_t i = 6; i < N_READINGS; i++) {
      if (readings[i] > MAX_DISTANCE) {                     // No hand present.
        noHandMeasurements++;                               // Keep track of how many in a row.
      }
      else {
        noHandMeasurements = 0;                             // Reset the count if there's a hand at this point.
      }
      if (noHandMeasurements == 3) {                        // Three subsequent no-hand measurements, so
        swipe = true;                                       // it's a swipe!
        break;                                              // End the loop, we're done.
      }
    }
  }
  bitWrite(state, SWIPE, swipe);

  static uint32_t lastMotionDetected;
  static bool haveMotion;
  static bool eepromUpdated;
  if (state == 0 &&
      haveMotion == true) {                                         // No more motion detected.
    haveMotion = false;
    lastMotionDetected = millis();
  }
  else {
    haveMotion = true;
    eepromUpdated = false;
  }

  if (haveMotion == false &&
    eepromUpdated == false &&
      millis() - lastMotionDetected > 5000) {
    updateEEPROM();
    eepromUpdated = true;
  }
}

void getReading() {
  uint16_t reading = handSensor.readRangeContinuousMillimeters();
  for (uint8_t i = N_READINGS - 1; i > 0; i--) {
    readings[i] = readings [i - 1];                         // Move all values one up.
  }
  readings[0] = reading;
}
