#include <EEPROM.h>

void readEEPROM() {
  EEPROM.get(32, activeFadeSpeed);                          // uint16_t
  EEPROM.get(34, transitionSpeed);                          // uint16_t
  EEPROM.get(36, musicHue);                                 // uint8_t
  EEPROM.get(37, musicSaturation);                          // uint8_t
  EEPROM.get(38, musicVolume);                              // uint8_t
  if (musicVolume > 30) {                                   // Fresh EEPROM, or just messed up settings. Set defaults.
    activeFadeSpeed = 6000;
    transitionSpeed = 2000;
    musicVolume = 25;
    updateEEPROM();
  }
}

void updateEEPROM() {
  EEPROM.put(32, activeFadeSpeed);                          // uint16_t
  EEPROM.put(34, transitionSpeed);                          // uint16_t
  EEPROM.put(36, musicHue);                                 // uint8_t
  EEPROM.put(37, musicSaturation);                          // uint8_t
  EEPROM.put(38, musicVolume);                              // uint8_t
}
