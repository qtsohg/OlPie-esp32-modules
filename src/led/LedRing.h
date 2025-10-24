#pragma once

#include <Arduino.h>
#include <NeoPixelBusLg.h>

namespace espmods::led {

class LedRing {
 public:
  LedRing(uint8_t pin, uint16_t count, uint8_t brightness);
  void begin();
  void showIdleGlow(uint32_t millisNow);
  void showProgress(uint32_t elapsedSeconds, uint32_t totalSeconds);
  void showConfetti(uint32_t millisNow);
  void clear();
  void loop();
  void setGradientColors(uint32_t startColor, uint32_t endColor);
  void setBrushingActive(bool active);

 private:
  void renderProgressFrame(uint32_t millisNow);
  RgbColor colorFromHex(uint32_t color) const;
  RgbColor gradientColor(float fraction, float intensity) const;

  NeoPixelBusLg<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod> strip_;
  uint16_t count_;
  uint8_t brightness_;
  uint32_t confettiUntil_ = 0;
  uint32_t idlePulseStart_ = 0;
  bool idlePulseIncreasing_ = true;
  RgbColor gradientStart_;
  RgbColor gradientEnd_;
  bool progressMode_ = false;
  float progressRatio_ = 0.0f;
  bool brushingActive_ = false;
  float highlightPosition_ = 0.0f;
  int8_t highlightDirection_ = 1;
  uint32_t lastAnimationUpdate_ = 0;
};
}
