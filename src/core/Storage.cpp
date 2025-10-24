#include "Storage.h"
namespace espmods::core {


void Storage::begin() { prefs_.begin("toothbrush", false); }

void Storage::saveStreak(uint8_t profileIndex, uint32_t streak) {
  prefs_.putULong(String("streak_" + String(profileIndex)).c_str(), streak);
}

uint32_t Storage::loadStreak(uint8_t profileIndex) {
  return prefs_.getULong(String("streak_" + String(profileIndex)).c_str(), 0);
}

void Storage::saveProfileIndex(uint8_t index) {
  prefs_.putUChar("profile_index", index);
}

uint8_t Storage::loadProfileIndex() { return prefs_.getUChar("profile_index", 0); }

void Storage::saveThreshold(uint8_t profileIndex, float threshold) {
  prefs_.putFloat(String("threshold_" + String(profileIndex)).c_str(), threshold);
}

float Storage::loadThreshold(uint8_t profileIndex, float defaultValue) {
  return prefs_.getFloat(String("threshold_" + String(profileIndex)).c_str(),
                         defaultValue);
}
}