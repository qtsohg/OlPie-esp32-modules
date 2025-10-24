#include "LedRing.h"

#include <cmath>

namespace espmods::led {

LedRing::LedRing(uint8_t pin, uint16_t count, uint8_t brightness)
    : strip_(count, pin),
      count_(count),
      brightness_(brightness),
      gradientStart_(brightness, brightness, brightness),
      gradientEnd_(brightness, brightness, brightness) {}

void LedRing::begin() {
  strip_.Begin();
  strip_.Show();
  idlePulseStart_ = millis();
}


void LedRing::showIdleGlow(uint32_t millisNow) {
  progressMode_ = false;
  uint32_t period = 4000;
  uint32_t elapsed = (millisNow - idlePulseStart_) % period;
  float phase = static_cast<float>(elapsed) / period;
  float intensity = sinf(phase * TWO_PI) * 0.5f + 0.5f;
  for (uint16_t i = 0; i < count_; ++i) {
    float fraction = count_ <= 1 ? 0.0f : static_cast<float>(i) / (count_ - 1);
    strip_.SetPixelColor(i, gradientColor(fraction, intensity));
  }
  strip_.Show();
}

void LedRing::showProgress(uint32_t elapsedSeconds, uint32_t totalSeconds) {
  float ratio = totalSeconds > 0 ? (float)elapsedSeconds / totalSeconds : 0.0f;
  bool wasInProgress = progressMode_;
  progressMode_ = true;
  progressRatio_ = constrain(ratio, 0.0f, 1.0f);
  if (!wasInProgress || progressRatio_ <= 0.0f) {
    highlightPosition_ = 0.0f;
    highlightDirection_ = 1;
  }
  lastAnimationUpdate_ = millis();
  renderProgressFrame(lastAnimationUpdate_);
}

void LedRing::showConfetti(uint32_t millisNow) {
  progressMode_ = false;
  confettiUntil_ = millisNow + 3000;
}

void LedRing::clear() {
  progressMode_ = false;
  for (uint16_t i = 0; i < count_; ++i) {
    strip_.SetPixelColor(i, RgbColor(0));
  }
  strip_.Show();
}

void LedRing::loop() {
  if (confettiUntil_ == 0) {
    if (progressMode_) {
      renderProgressFrame(millis());
    }
    return;
  }
  if (millis() > confettiUntil_) {
    confettiUntil_ = 0;
    clear();
    return;
  }
  for (uint16_t i = 0; i < count_; ++i) {
    uint8_t r = random(0, brightness_ + 1);
    uint8_t g = random(0, brightness_ + 1);
    uint8_t b = random(0, brightness_ + 1);
    strip_.SetPixelColor(i, RgbColor(r, g, b));
  }

  strip_.Show();
}

void LedRing::setGradientColors(uint32_t startColor, uint32_t endColor) {
  gradientStart_ = colorFromHex(startColor);
  gradientEnd_ = colorFromHex(endColor);
  if (progressMode_) {
    renderProgressFrame(millis());
  } else {
    showIdleGlow(millis());
  }
}

void LedRing::setBrushingActive(bool active) {
  if (brushingActive_ == active) {
    return;
  }
  brushingActive_ = active;
  lastAnimationUpdate_ = millis();
}

RgbColor LedRing::colorFromHex(uint32_t color) const {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  return RgbColor(r, g, b);
}

RgbColor LedRing::gradientColor(float fraction, float intensity) const {
  fraction = constrain(fraction, 0.0f, 1.0f);
  intensity = constrain(intensity, 0.0f, 1.0f);
  float brightnessScale = static_cast<float>(brightness_) / 255.0f;
  float startR = gradientStart_.R * brightnessScale;
  float startG = gradientStart_.G * brightnessScale;
  float startB = gradientStart_.B * brightnessScale;
  float endR = gradientEnd_.R * brightnessScale;
  float endG = gradientEnd_.G * brightnessScale;
  float endB = gradientEnd_.B * brightnessScale;
  float r = (startR + (endR - startR) * fraction) * intensity;
  float g = (startG + (endG - startG) * fraction) * intensity;
  float b = (startB + (endB - startB) * fraction) * intensity;
  return RgbColor(static_cast<uint8_t>(roundf(r)), static_cast<uint8_t>(roundf(g)),
                  static_cast<uint8_t>(roundf(b)));
}

void LedRing::renderProgressFrame(uint32_t millisNow) {
  float progressPixels = progressRatio_ * count_;
  if (progressPixels < 0.0f) progressPixels = 0.0f;
  uint16_t fullPixels = static_cast<uint16_t>(floorf(progressPixels));
  if (fullPixels > count_) fullPixels = count_;
  float partialPixel = progressPixels - fullPixels;
  uint16_t effectivePixels = fullPixels;
  if (partialPixel > 0.01f && effectivePixels < count_) {
    effectivePixels += 1;
  }
  if (progressRatio_ > 0.0f && effectivePixels == 0) {
    effectivePixels = 1;
  }

  if (brushingActive_ && effectivePixels > 1) {
    float elapsed = (millisNow - lastAnimationUpdate_) / 1000.0f;
    float speed = 3.0f;  // LEDs per second
    highlightPosition_ += highlightDirection_ * elapsed * speed;
    float maxPosition = static_cast<float>(effectivePixels - 1);
    if (highlightPosition_ >= maxPosition) {
      highlightPosition_ = maxPosition;
      highlightDirection_ = -1;
    } else if (highlightPosition_ <= 0.0f) {
      highlightPosition_ = 0.0f;
      highlightDirection_ = 1;
    }
  } else {
    highlightPosition_ = 0.0f;
    highlightDirection_ = 1;
  }
  lastAnimationUpdate_ = millisNow;

  for (uint16_t i = 0; i < count_; ++i) {
    float intensity = 0.0f;
    if (i < fullPixels) {
      intensity = 1.0f;
    } else if (i == fullPixels && partialPixel > 0.0f && fullPixels < count_) {
      intensity = partialPixel;
    }

    if (intensity > 0.0f) {
      float fraction = effectivePixels <= 1 ? 0.0f : static_cast<float>(i) / (effectivePixels - 1);
      float modulation = 1.0f;
      if (brushingActive_ && effectivePixels > 0) {
        float distance = fabsf(static_cast<float>(i) - highlightPosition_);
        if (distance < 1.0f) {
          float highlight = 1.0f - distance;
          modulation = 0.6f + 0.4f * highlight;
        } else {
          modulation = 0.6f;
        }
      }
      strip_.SetPixelColor(i, gradientColor(fraction, intensity * modulation));
    } else {
      strip_.SetPixelColor(i, RgbColor(0));
    }
  }
  strip_.Show();
}
}