#include "OledUi.h"

namespace espmods::display {

OledUi::OledUi(uint8_t sda, uint8_t scl, uint8_t address)
    : display_(U8G2_R0, /* reset=*/U8X8_PIN_NONE) {
  display_.setI2CAddress(address << 1);
  display_.setBusClock(400000);
}

void OledUi::begin() {
  display_.begin();
  display_.setFont(u8g2_font_ncenB08_tr);
  target_ = rendered_ = UiState{};
}

void OledUi::showIdle(const String &profileName, uint32_t streak) {
  target_.line1 = "Profile: " + profileName;
  target_.line2 = "Streak: " + String(streak);
  target_.line3 = "Ready to brush!";
  target_.eyes = true;
}

void OledUi::showBrushing(const String &profileName, uint32_t remainingSeconds) {
  uint32_t minutes = remainingSeconds / 60;
  uint32_t seconds = remainingSeconds % 60;
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02u:%02u", minutes, seconds);
  target_.line1 = profileName;
  target_.line2 = "Brush Time";
  target_.line3 = buffer;
  target_.eyes = false;
}

void OledUi::showPaused() {
  target_.line1 = "Paused";
  target_.line2 = "Tap to resume";
  target_.line3 = "";
  target_.eyes = false;
}

void OledUi::showReady() {
  target_.line1 = "Ready....";
  target_.line2 = "";
  target_.line3 = "";
  target_.eyes = false;
}

void OledUi::showFinish() {
  target_.line1 = "Great job!";
  target_.line2 = "Brush complete";
  target_.line3 = "";
  target_.eyes = true;
}

void OledUi::showCustom(const String &line1, const String &line2, const String &line3, bool eyes) {
  target_.line1 = line1;
  target_.line2 = line2;
  target_.line3 = line3;
  target_.eyes = eyes;
}

void OledUi::render() {
  display_.clearBuffer();
  display_.setCursor(0, 14);
  display_.print(target_.line1);
  display_.setCursor(0, 30);
  display_.print(target_.line2);
  display_.setCursor(0, 46);
  display_.print(target_.line3);
  if (target_.eyes) {
    display_.drawDisc(32, 54, 6);
    display_.drawDisc(96, 54, 6);
  }
  display_.sendBuffer();
  rendered_ = target_;
}

void OledUi::loop() {
  if (target_.line1 != rendered_.line1 || target_.line2 != rendered_.line2 ||
      target_.line3 != rendered_.line3 || target_.eyes != rendered_.eyes) {
    render();
  }
}
}