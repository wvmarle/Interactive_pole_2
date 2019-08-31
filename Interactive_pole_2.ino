#include <Wire.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <FastLED.h>

//#define TEST_PROX_SENSOR

const uint8_t LED_PIN = 5;
const uint8_t PROXIMITY_SENSOR_PIN = A1;
const uint8_t BRIGHTNESS_SENSOR_PIN = A0;

SoftwareSerial mySoftwareSerial(8, 7); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// Bits to set when a specific movement is detected.
#define MOTION_DOWN     0                                   // Hand moving down.
#define MOTION_UP       1                                   // Hand moving up.
#define HAND_PRESENTED  2                                   // Hand presented above the sensor (state change).
#define HAND_REMOVED    3                                   // Hand removed from above the sensor (state change).
#define SWIPE           4                                   // Hand swiped over the sensor.
#define HAND_PRESENT    5                                   // Hand held above the sensor.
uint8_t motionState;
bool musicPlaying;

const uint8_t N_READINGS = 20;                              // We keep the latest 20 readings for hand gesture detection.
uint16_t readings[N_READINGS];
const uint16_t MAX_DISTANCE = 750;                          // If distance greater than this number (in mm), no hand present.
const uint16_t MIN_DISTANCE = 50;                           // If distance less than this number (in mm), sensor is covered.

const uint32_t PROXIMITY_TIMEOUT = 15000;                   // How long to remain "active" after motion sensor says there's no movement nearby.
const uint32_t MUSIC_PLAYING_TIMEOUT = 30000;               // How long to remain "active" after motion sensor says there's no movement nearby, and we have music playing.
bool proximityDetected = false;

uint8_t musicVolume;                                        // The volume of the music: 0-30.

uint16_t activeFadeSpeed, transitionSpeed;

// Colour when the pole is idle - not fading. Same for all poles.
uint8_t idleHue = 0;                                        // Red.
uint8_t idleSaturation = 0;                                 // White.
const uint8_t value = 255;                                  // Maximum brightness - always.

// Colours when music plays - not fading. Set through the menu pole by pole.
uint8_t musicHue, musicSaturation; //, musicValue;

char buf[80];

void setup() {
  Serial.begin(115200);
  Wire.begin();
  readEEPROM();
  initLEDs();
  initProximitySensor();
  initHandSensor();
  initMusic();
  FastLED.setBrightness(200);
}

void loop() {
  handleHandSensor();
  handleMusic();
  handleProximitySensor();
  handleLEDs(); 
  handleSerialUI();
}
