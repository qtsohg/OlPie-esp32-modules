#include "Storage.h"
namespace espmods::core {


void Storage::begin() { prefs_.begin("halloweeninator", false); }

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

// Generic storage methods
void Storage::saveUInt32(const char* key, uint32_t value) {
  prefs_.putULong(key, value);
}

uint32_t Storage::loadUInt32(const char* key, uint32_t defaultValue) {
  return prefs_.getULong(key, defaultValue);
}

void Storage::saveFloat(const char* key, float value) {
  prefs_.putFloat(key, value);
}

float Storage::loadFloat(const char* key, float defaultValue) {
  return prefs_.getFloat(key, defaultValue);
}

}