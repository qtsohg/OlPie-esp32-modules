#include "LedStrip.h"
#include <cmath>

namespace espmods::led {

LedStrip::LedStrip(uint8_t pin, uint16_t count, uint8_t brightness)
    : strip_(count, pin),
      count_(count),
      brightness_(brightness),
      effectStartTime_(0),
      lastUpdate_(0),
      wavePosition_(0.0f),
      lastFlickerUpdate_(0),
      lightningStartTime_(0),
      lightningActive_(false),
      lastSparkleUpdate_(0),
      sparklePixels_(nullptr) {
  
  // Allocate sparkle tracking array
  sparklePixels_ = new uint8_t[count_];
  memset(sparklePixels_, 0, count_);
}

LedStrip::~LedStrip() {
  delete[] sparklePixels_;
}

void LedStrip::begin() {
  strip_.Begin();
  off();
  effectStartTime_ = millis();
}

void LedStrip::update() {
  uint32_t now = millis();
  lastUpdate_ = now;
  
  switch (currentEffect_.effect) {
    case LedEffect::Off:
      renderOff();
      break;
    case LedEffect::SolidColor:
      renderSolidColor();
      break;
    case LedEffect::Pulse:
      renderPulse();
      break;
    case LedEffect::GradientPulse:
      renderGradientPulse();
      break;
    case LedEffect::RandomFlicker:
      renderRandomFlicker();
      break;
    case LedEffect::Strobe:
      renderStrobe();
      break;
    case LedEffect::ColorWave:
      renderColorWave();
      break;
    case LedEffect::Fire:
      renderFire();
      break;
    case LedEffect::Lightning:
      renderLightning();
      break;
    case LedEffect::Rainbow:
      renderRainbow();
      break;
    case LedEffect::Sparkle:
      renderSparkle();
      break;
  }
  
  strip_.Show();
}

void LedStrip::setEffect(const LedEffectConfig& config) {
  currentEffect_ = config;
  effectStartTime_ = millis();
  
  // Reset effect-specific state
  wavePosition_ = 0.0f;
  lightningActive_ = false;
  memset(sparklePixels_, 0, count_);
}

void LedStrip::setSolidColor(uint32_t color) {
  LedEffectConfig config;
  config.effect = LedEffect::SolidColor;
  config.primaryColor = color;
  setEffect(config);
}

void LedStrip::off() {
  LedEffectConfig config;
  config.effect = LedEffect::Off;
  setEffect(config);
}

void LedStrip::setBrightness(uint8_t brightness) {
  brightness_ = brightness;
}

// Quick effect methods
void LedStrip::pulseColor(uint32_t color, uint32_t speed) {
  LedEffectConfig config;
  config.effect = LedEffect::Pulse;
  config.primaryColor = color;
  config.speed = speed;
  setEffect(config);
}

void LedStrip::strobe(uint32_t color, uint32_t speed) {
  LedEffectConfig config;
  config.effect = LedEffect::Strobe;
  config.primaryColor = color;
  config.speed = speed;
  setEffect(config);
}

void LedStrip::gradientPulse(uint32_t color1, uint32_t color2, uint32_t speed) {
  LedEffectConfig config;
  config.effect = LedEffect::GradientPulse;
  config.primaryColor = color1;
  config.secondaryColor = color2;
  config.speed = speed;
  setEffect(config);
}

void LedStrip::randomFlicker(uint32_t baseColor, uint8_t intensity) {
  LedEffectConfig config;
  config.effect = LedEffect::RandomFlicker;
  config.primaryColor = baseColor;
  config.intensity = intensity;
  config.speed = 50; // Fast flicker updates
  setEffect(config);
}

void LedStrip::lightning(uint32_t color) {
  LedEffectConfig config;
  config.effect = LedEffect::Lightning;
  config.primaryColor = color;
  config.speed = 2000; // Lightning cycle time
  setEffect(config);
}

void LedStrip::fire(uint8_t intensity) {
  LedEffectConfig config;
  config.effect = LedEffect::Fire;
  config.primaryColor = 0xFF4400; // Orange-red
  config.secondaryColor = 0xFF0000; // Red
  config.intensity = intensity;
  config.speed = 100; // Fast fire updates
  setEffect(config);
}

void LedStrip::rainbow(uint32_t speed) {
  LedEffectConfig config;
  config.effect = LedEffect::Rainbow;
  config.speed = speed;
  setEffect(config);
}

void LedStrip::colorWave(uint32_t color1, uint32_t color2, uint32_t speed) {
  LedEffectConfig config;
  config.effect = LedEffect::ColorWave;
  config.primaryColor = color1;
  config.secondaryColor = color2;
  config.speed = speed;
  setEffect(config);
}

void LedStrip::sparkle(uint32_t color, uint8_t density) {
  LedEffectConfig config;
  config.effect = LedEffect::Sparkle;
  config.primaryColor = color;
  config.intensity = density;
  config.speed = 100; // Sparkle update rate
  setEffect(config);
}

// Effect rendering methods
void LedStrip::renderOff() {
  for (uint16_t i = 0; i < count_; i++) {
    strip_.SetPixelColor(i, RgbColor(0, 0, 0));
  }
}

void LedStrip::renderSolidColor() {
  RgbColor color = colorFromHex(currentEffect_.primaryColor);
  for (uint16_t i = 0; i < count_; i++) {
    strip_.SetPixelColor(i, color);
  }
}

void LedStrip::renderPulse() {
  float intensity = sineWave(lastUpdate_ - effectStartTime_, currentEffect_.speed);
  RgbColor baseColor = colorFromHex(currentEffect_.primaryColor);
  
  for (uint16_t i = 0; i < count_; i++) {
    RgbColor pulseColor(
      applyBrightness(baseColor.R * intensity),
      applyBrightness(baseColor.G * intensity),
      applyBrightness(baseColor.B * intensity)
    );
    strip_.SetPixelColor(i, pulseColor);
  }
}

void LedStrip::renderGradientPulse() {
  float phase = sineWave(lastUpdate_ - effectStartTime_, currentEffect_.speed);
  RgbColor color1 = colorFromHex(currentEffect_.primaryColor);
  RgbColor color2 = colorFromHex(currentEffect_.secondaryColor);
  
  for (uint16_t i = 0; i < count_; i++) {
    float position = count_ > 1 ? (float)i / (count_ - 1) : 0.0f;
    RgbColor gradientColor = blendColors(color1, color2, position);
    
    RgbColor pulseColor(
      applyBrightness(gradientColor.R * phase),
      applyBrightness(gradientColor.G * phase),
      applyBrightness(gradientColor.B * phase)
    );
    strip_.SetPixelColor(i, pulseColor);
  }
}

void LedStrip::renderRandomFlicker() {
  if (lastUpdate_ - lastFlickerUpdate_ >= currentEffect_.speed) {
    lastFlickerUpdate_ = lastUpdate_;
    
    RgbColor baseColor = colorFromHex(currentEffect_.primaryColor);
    
    for (uint16_t i = 0; i < count_; i++) {
      // Random flicker intensity
      float flicker = 0.5f + (random(0, currentEffect_.intensity) / 255.0f) * 0.5f;
      
      RgbColor flickerColor(
        applyBrightness(baseColor.R * flicker),
        applyBrightness(baseColor.G * flicker),
        applyBrightness(baseColor.B * flicker)
      );
      strip_.SetPixelColor(i, flickerColor);
    }
  }
}

void LedStrip::renderStrobe() {
  uint32_t elapsed = lastUpdate_ - effectStartTime_;
  uint32_t cycle = elapsed % currentEffect_.speed;
  bool on = cycle < (currentEffect_.speed / 2);
  
  RgbColor color = on ? colorFromHex(currentEffect_.primaryColor) : RgbColor(0, 0, 0);
  
  for (uint16_t i = 0; i < count_; i++) {
    strip_.SetPixelColor(i, color);
  }
}

void LedStrip::renderColorWave() {
  uint32_t elapsed = lastUpdate_ - effectStartTime_;
  float waveProgress = (float)(elapsed % currentEffect_.speed) / currentEffect_.speed;
  
  RgbColor color1 = colorFromHex(currentEffect_.primaryColor);
  RgbColor color2 = colorFromHex(currentEffect_.secondaryColor);
  
  for (uint16_t i = 0; i < count_; i++) {
    float position = (float)i / count_;
    float wavePhase = fmod(position + waveProgress, 1.0f);
    float intensity = sin(wavePhase * TWO_PI) * 0.5f + 0.5f;
    
    RgbColor waveColor = blendColors(color1, color2, intensity);
    strip_.SetPixelColor(i, waveColor);
  }
}

void LedStrip::renderFire() {
  if (lastUpdate_ - lastFlickerUpdate_ >= currentEffect_.speed) {
    lastFlickerUpdate_ = lastUpdate_;
    
    for (uint16_t i = 0; i < count_; i++) {
      // Create fire effect with random oranges and reds
      uint8_t red = random(100, 255);
      uint8_t green = random(0, red / 2);
      uint8_t blue = 0;
      
      // Add some flicker
      float flicker = 0.7f + (random(0, 30) / 100.0f);
      
      RgbColor fireColor(
        applyBrightness(red * flicker),
        applyBrightness(green * flicker),
        applyBrightness(blue)
      );
      strip_.SetPixelColor(i, fireColor);
    }
  }
}

void LedStrip::renderLightning() {
  uint32_t elapsed = lastUpdate_ - effectStartTime_;
  
  // Lightning strikes every few seconds with random timing
  if (!lightningActive_ && elapsed > 1000 && random(0, 100) < 2) {
    lightningActive_ = true;
    lightningStartTime_ = lastUpdate_;
  }
  
  if (lightningActive_) {
    uint32_t lightningElapsed = lastUpdate_ - lightningStartTime_;
    
    if (lightningElapsed < 100) {
      // Bright flash
      RgbColor lightningColor = colorFromHex(currentEffect_.primaryColor);
      for (uint16_t i = 0; i < count_; i++) {
        strip_.SetPixelColor(i, lightningColor);
      }
    } else if (lightningElapsed < 150) {
      // Brief darkness
      for (uint16_t i = 0; i < count_; i++) {
        strip_.SetPixelColor(i, RgbColor(0, 0, 0));
      }
    } else if (lightningElapsed < 200) {
      // Second flash (dimmer)
      RgbColor lightningColor = colorFromHex(currentEffect_.primaryColor);
      lightningColor.R /= 2;
      lightningColor.G /= 2;
      lightningColor.B /= 2;
      for (uint16_t i = 0; i < count_; i++) {
        strip_.SetPixelColor(i, lightningColor);
      }
    } else {
      // End lightning
      lightningActive_ = false;
      effectStartTime_ = lastUpdate_; // Reset cycle
      for (uint16_t i = 0; i < count_; i++) {
        strip_.SetPixelColor(i, RgbColor(0, 0, 0));
      }
    }
  } else {
    // Dark between lightning
    for (uint16_t i = 0; i < count_; i++) {
      strip_.SetPixelColor(i, RgbColor(0, 0, 0));
    }
  }
}

void LedStrip::renderRainbow() {
  uint32_t elapsed = lastUpdate_ - effectStartTime_;
  float hueOffset = (float)(elapsed % currentEffect_.speed) / currentEffect_.speed;
  
  for (uint16_t i = 0; i < count_; i++) {
    float position = (float)i / count_;
    float hue = fmod(position + hueOffset, 1.0f) * 360.0f;
    
    // Convert HSV to RGB (simple approximation)
    float c = 1.0f;
    float x = 1.0f - abs(fmod(hue / 60.0f, 2.0f) - 1.0f);
    float r, g, b;
    
    if (hue < 60) { r = c; g = x; b = 0; }
    else if (hue < 120) { r = x; g = c; b = 0; }
    else if (hue < 180) { r = 0; g = c; b = x; }
    else if (hue < 240) { r = 0; g = x; b = c; }
    else if (hue < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    RgbColor rainbowColor(
      applyBrightness(r * 255),
      applyBrightness(g * 255),
      applyBrightness(b * 255)
    );
    strip_.SetPixelColor(i, rainbowColor);
  }
}

void LedStrip::renderSparkle() {
  if (lastUpdate_ - lastSparkleUpdate_ >= currentEffect_.speed) {
    lastSparkleUpdate_ = lastUpdate_;
    
    RgbColor sparkleColor = colorFromHex(currentEffect_.primaryColor);
    
    // Fade existing sparkles
    for (uint16_t i = 0; i < count_; i++) {
      if (sparklePixels_[i] > 0) {
        sparklePixels_[i] = max(0, sparklePixels_[i] - 20);
        float intensity = sparklePixels_[i] / 255.0f;
        RgbColor fadeColor(
          applyBrightness(sparkleColor.R * intensity),
          applyBrightness(sparkleColor.G * intensity),
          applyBrightness(sparkleColor.B * intensity)
        );
        strip_.SetPixelColor(i, fadeColor);
      } else {
        strip_.SetPixelColor(i, RgbColor(0, 0, 0));
      }
    }
    
    // Add new sparkles
    uint8_t newSparkles = random(0, currentEffect_.intensity / 10 + 1);
    for (uint8_t s = 0; s < newSparkles; s++) {
      uint16_t pixel = random(0, count_);
      sparklePixels_[pixel] = 255;
    }
  }
}

// Utility methods
RgbColor LedStrip::colorFromHex(uint32_t color) const {
  uint8_t r = applyBrightness((color >> 16) & 0xFF);
  uint8_t g = applyBrightness((color >> 8) & 0xFF);
  uint8_t b = applyBrightness(color & 0xFF);
  return RgbColor(r, g, b);
}

RgbColor LedStrip::blendColors(const RgbColor& color1, const RgbColor& color2, float ratio) const {
  ratio = constrain(ratio, 0.0f, 1.0f);
  return RgbColor(
    color1.R + (color2.R - color1.R) * ratio,
    color1.G + (color2.G - color1.G) * ratio,
    color1.B + (color2.B - color1.B) * ratio
  );
}

uint8_t LedStrip::applyBrightness(uint8_t value) const {
  return (value * brightness_) / 255;
}

float LedStrip::sineWave(uint32_t timeMs, uint32_t period) const {
  float phase = (float)(timeMs % period) / period;
  return sin(phase * TWO_PI) * 0.5f + 0.5f;
}

}  // namespace espmods::led