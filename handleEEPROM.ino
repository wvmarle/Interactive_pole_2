#include <EEPROM.h>

void readEEPROM() {
  EEPROM.get(0, activeFadeSpeed);                           // uint16_t
  EEPROM.get(10, transitionSpeed);                          // uint16_t
//  EEPROM.get(14, musicRed);                                 // uint8_t
//  EEPROM.get(16, musicGreen);                               // uint8_t
//  EEPROM.get(18, musicBlue);                                // uint8_t
  EEPROM.get(14, musicHue);                                 // uint8_t
  EEPROM.get(20, musicVolume);                              // uint8_t
  if (musicVolume > 30) {                                   // Fresh EEPROM, or just messed up settings. Set defaults.
    activeFadeSpeed = 6000;
    transitionSpeed = 2000;

    musicVolume = 25;
    updateEEPROM();
  }
}

void updateEEPROM() {
  EEPROM.put(0, activeFadeSpeed);                           // uint16_t
  EEPROM.put(10, transitionSpeed);                          // uint16_t
//  EEPROM.put(14, musicRed);                                 // uint8_t
//  EEPROM.put(16, musicGreen);                               // uint8_t
//  EEPROM.put(18, musicBlue);                                // uint8_t
  EEPROM.put(14, musicHue);                                 // uint8_t
  EEPROM.put(20, musicVolume);                              // uint8_t
}
