#include <FastLED.h>
const uint8_t NUM_LEDS = 120;                               // The total number of LEDs in the strip (or a little less than this, as they're all the same colour anyway).
CRGB leds[NUM_LEDS];                                        // Define the array of leds
uint8_t hue, saturation;                                    // The current colours.

enum LEDStates {
  LED_IDLE,                                                 // Show idle.
  LED_TRANSITION_TO_ACTIVE,                                 // Fade to active colour [0].
  LED_TRANSITION_TO_MUSIC,                                  // Fade to music playing colour.
  LED_TRANSITION_TO_IDLE,                                   // Fade to idle colour.
  LED_ACTIVE_FADE_UP,                                       // Fade from active colour [0] to active colour [1].
  LED_ACTIVE_FADE_DOWN,                                     // Fade from active colour [1] to active colour [0].
  LED_MUSIC,                                                // Show music playing colour.
};
LEDStates LEDState = LED_IDLE;

void initLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  if (proximityDetected) {                                  // We set the "wrong" transition here to trigger fading.
    LEDState = LED_TRANSITION_TO_IDLE;
  }
  else {
    LEDState = LED_TRANSITION_TO_ACTIVE;
  }
  CRGB rgb = CRGB(0, 0, 0);                                 // Switch off the LEDs.
  FastLED.showColor(rgb);
}

void handleLEDs() {
  static uint32_t lastFadeTime;                             // When a fade cycle (0 to 1 to 0) started.
  static uint8_t oldHue, oldSaturation;                     // Existing colour hue & saturation - when doing transition fade.
  static uint8_t fadeStep;
  static uint16_t fadeSpeed = activeFadeSpeed;
  static uint8_t fadeHue;
  static uint8_t fadeSaturation;
  static uint8_t nSteps;
  static bool oldProximityDetected;
  static bool oldMusicPlaying;
  static bool updateColour;

  // Handle proximity sensor state.
  if (proximityDetected != oldProximityDetected &&          // Change of state; act upon this,
      musicPlaying == false) {                              // but only when we're not currently playing music!
    oldProximityDetected = proximityDetected;
    lastFadeTime = millis();                                // Starting a fade event.
    oldHue = hue;                                           // Keep track of where we started.
    oldSaturation = saturation;
    if (proximityDetected) {                                // Become active.
      LEDState = LED_ACTIVE_FADE_UP;
      fadeSpeed = activeFadeSpeed;
    }
    else if (musicPlaying == false) {
      LEDState = LED_TRANSITION_TO_IDLE;
      fadeSpeed = transitionSpeed;
      fadeHue = idleHue;
      fadeSaturation = idleSaturation;
    }
    fadeStep = 0;
    fadeSpeed = transitionSpeed;                            // How long the transition should take.
  }

  // Handle start/stop of music playing.
  if (musicPlaying != oldMusicPlaying) {
    oldMusicPlaying = musicPlaying;
    oldHue = hue;                                           // Keep track of where we started.
    oldSaturation = saturation;
    if (musicPlaying) {                                     // Music just started.
      LEDState = LED_TRANSITION_TO_MUSIC;                   // Transition fade to music colour.
      fadeHue = musicHue;
      fadeSaturation = musicSaturation;
      fadeSpeed = transitionSpeed;                          // How long the transition should take.
    }
    else if (proximityDetected) {                           // Music just stopped, still have proximity.
      LEDState = LED_ACTIVE_FADE_DOWN;                      // Transition fade to active colour.
      fadeSpeed = activeFadeSpeed;                          // How long the transition should take.
    }
    else {
      LEDState = LED_TRANSITION_TO_IDLE;                    // Transition fade to idle colour.
      fadeHue = idleHue;
      fadeSaturation = idleSaturation;
      fadeSpeed = transitionSpeed;                          // How long the transition should take.
    }
    fadeStep = 0;
  }
  nSteps = min(fadeSpeed / 50, 255);                        // Don't fade too fast, but also no more than 256 total steps. It takes about 32 ms to transmit data to 100 LEDs!
  uint16_t fadeStepTime;
  if (nSteps == 255) {
    fadeStepTime = fadeSpeed / nSteps;
  }
  else {
    fadeStepTime = 50;
  }
  if (millis() - lastFadeTime > (float)fadeStep * (float)fadeStepTime) {
    fadeStep++;
    switch (LEDState) {
      case LED_IDLE:
        break;

      case LED_TRANSITION_TO_IDLE:
      case LED_TRANSITION_TO_ACTIVE:
      case LED_TRANSITION_TO_MUSIC:
        setLEDs(oldHue, fadeHue,
                oldSaturation, fadeSaturation,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {                           // We reached the end of the sequence.
          if (LEDState == LED_TRANSITION_TO_IDLE) {
            LEDState = LED_IDLE;
            setLEDs(idleHue, 0,
                    idleSaturation, 0,
                    0, 0);
          }
          else if (LEDState == LED_TRANSITION_TO_ACTIVE) {
            LEDState = LED_ACTIVE_FADE_UP;
            fadeSpeed = activeFadeSpeed;                    // How long the transition should take.
          }
          else if (LEDState == LED_TRANSITION_TO_MUSIC) {
            LEDState = LED_MUSIC;
            setLEDs(musicHue, 0,
                    musicSaturation, 0,
                    0, 0);
          }
        }
        break;

      case LED_ACTIVE_FADE_UP:
        setLEDs(idleHue, musicHue,
                idleSaturation, musicSaturation,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          LEDState = LED_ACTIVE_FADE_DOWN;                // Start fading down (colour 1 to 0).
          fadeSpeed = activeFadeSpeed;
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;

      case LED_ACTIVE_FADE_DOWN:
        setLEDs(musicHue, idleHue,
                musicSaturation, idleSaturation,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          LEDState = LED_ACTIVE_FADE_UP;                    // Start fading up (colour 0 to 1).
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;

      case LED_MUSIC:
        if (updateColour) {
          updateColour = false;
          setLEDs(musicHue, 0,
                  musicSaturation, 0,
                  0, 0);
        }
        break;
    }
  }

  // Change colours depending on the hand sensor.
  // Holding hand above the sensor: rotate through the colour spectrum.
  // Moving hand up/down: increase/decrease saturation.

  static uint32_t lastChange;
  if (millis() - lastChange > 20) {
    lastChange = millis();
    if (bitRead(motionState, MOTION_UP)) {
      Serial.print(F("Motion up - "));
      if (musicSaturation < 255) {
        musicSaturation++;
        updateColour = true;
        Serial.print(F("S: "));
        Serial.println(musicSaturation);
      }
    }
    if (bitRead(motionState, MOTION_DOWN)) {
      Serial.print(F("Motion down - "));
      if (musicSaturation > 0) {
        musicSaturation--;
        updateColour = true;
        Serial.print(F("S: "));
        Serial.println(musicSaturation);
      }
    }
    if (bitRead(motionState, HAND_PRESENT)) {
      Serial.println(F("Hand present."));
      musicHue++;
      updateColour = true;
      Serial.print(F("H: "));
      Serial.println(musicHue);
    }
  }
}

void setLEDs(uint8_t fromHue, uint8_t toHue,
             uint8_t fromSaturation, uint8_t toSaturation,
             uint8_t fadeStep, uint8_t nSteps) {
  if (nSteps == 0) {                                        // No fading; just set the LEDs to a colour.
    hue = fromHue;
    saturation = fromSaturation;
  }
  else {                                                    // Fade in progress: calculate the new colour.
    hue = (float)fromHue + ((float)toHue - (float)fromHue) * (float)fadeStep / (float)nSteps;
    saturation = (float)fromSaturation + ((float)toSaturation - (float)fromSaturation) * (float)fadeStep / (float)nSteps;
  }

  FastLED.showColor(CHSV(hue, saturation, value));
}
