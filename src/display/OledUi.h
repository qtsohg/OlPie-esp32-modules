#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "config.h"

namespace espmods::display {

struct UiState {
  String line1;
  String line2;
  String line3;
  bool eyes = true;
};

class OledUi {
 public:
  OledUi(uint8_t sda, uint8_t scl, uint8_t address);
  void begin();
  void showIdle(const String &profileName, uint32_t streak);
  void showBrushing(const String &profileName, uint32_t remainingSeconds);
  void showPaused();
  void showReady();
  void showFinish();
  void showCustom(const String &line1, const String &line2, const String &line3, bool eyes);
  void loop();

 private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_;
  UiState target_;
  UiState rendered_;
  void render();
};
}
