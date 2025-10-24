#pragma once

#include <Arduino.h>
#include <Preferences.h>

namespace espmods::core {


class Storage {
 public:
  void begin();
  void saveStreak(uint8_t profileIndex, uint32_t streak);
  uint32_t loadStreak(uint8_t profileIndex);
  void saveProfileIndex(uint8_t index);
  uint8_t loadProfileIndex();
 void saveThreshold(uint8_t profileIndex, float threshold);
  float loadThreshold(uint8_t profileIndex, float defaultValue);

 private:
  Preferences prefs_;
};
}