#pragma once

#include <Arduino.h>
#include <NeoPixelBusLg.h>

namespace espmods::led {

enum class LedEffect {
  Off,
  SolidColor,
  Pulse,
  GradientPulse,
  RandomFlicker,
  Strobe,
  ColorWave,
  Fire,
  Lightning,
  Rainbow,
  Sparkle
};

struct LedEffectConfig {
  LedEffect effect = LedEffect::Off;
  uint32_t primaryColor = 0xFF0000;    // Red default
  uint32_t secondaryColor = 0x000000;  // Black default
  uint32_t speed = 1000;               // Effect speed in milliseconds
  uint8_t intensity = 255;             // Effect intensity (0-255)
  bool reverse = false;                // Reverse direction if applicable
};

/**
 * @brief Generic LED strip controller with various effects
 * 
 * This class provides a simple wrapper around NeoPixelBus with common
 * LED effects suitable for Halloween displays, ambient lighting, and
 * other visual effects. The strip length is configurable at runtime.
 */
class LedStrip {
 public:
  /**
   * @brief Constructor for LED strip
   * @param pin GPIO pin connected to the LED strip data line
   * @param count Number of LEDs in the strip
   * @param brightness Default brightness (0-255)
   */
  LedStrip(uint8_t pin, uint16_t count, uint8_t brightness = 128);
  
  /**
   * @brief Destructor - cleans up allocated memory
   */
  ~LedStrip();
  
  /**
   * @brief Initialize the LED strip
   * Must be called in setup() before using other methods
   */
  void begin();
  
  /**
   * @brief Update the LED effects - call this in loop()
   */
  void update();
  
  /**
   * @brief Set the current effect and configuration
   * @param config Effect configuration struct
   */
  void setEffect(const LedEffectConfig& config);
  
  /**
   * @brief Quick method to set a solid color
   * @param color 24-bit RGB color (0xRRGGBB)
   */
  void setSolidColor(uint32_t color);
  
  /**
   * @brief Turn off all LEDs
   */
  void off();
  
  /**
   * @brief Set strip brightness
   * @param brightness Brightness level (0-255)
   */
  void setBrightness(uint8_t brightness);
  
  /**
   * @brief Get current brightness
   * @return Current brightness (0-255)
   */
  uint8_t getBrightness() const { return brightness_; }
  
  /**
   * @brief Get strip length
   * @return Number of LEDs in strip
   */
  uint16_t getLength() const { return count_; }

  // Quick effect methods for common use cases
  void pulseColor(uint32_t color, uint32_t speed = 2000);
  void strobe(uint32_t color, uint32_t speed = 100);
  void gradientPulse(uint32_t color1, uint32_t color2, uint32_t speed = 3000);
  void randomFlicker(uint32_t baseColor, uint8_t intensity = 50);
  void colorWave(uint32_t color1, uint32_t color2, uint32_t speed = 3000);
  void lightning(uint32_t color = 0xFFFFFF);
  void fire(uint8_t intensity = 128);
  void rainbow(uint32_t speed = 5000);
  void sparkle(uint32_t color, uint8_t density = 10);

 private:
  // Effect rendering methods
  void renderOff();
  void renderSolidColor();
  void renderPulse();
  void renderGradientPulse();
  void renderRandomFlicker();
  void renderStrobe();
  void renderColorWave();
  void renderFire();
  void renderLightning();
  void renderRainbow();
  void renderSparkle();
  
  // Utility methods
  RgbColor colorFromHex(uint32_t color) const;
  RgbColor blendColors(const RgbColor& color1, const RgbColor& color2, float ratio) const;
  uint8_t applyBrightness(uint8_t value) const;
  float sineWave(uint32_t timeMs, uint32_t period) const;
  
  // Hardware
  NeoPixelBusLg<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod> strip_;
  uint16_t count_;
  uint8_t brightness_;
  
  // Effect state
  LedEffectConfig currentEffect_;
  uint32_t effectStartTime_;
  uint32_t lastUpdate_;
  
  // Effect-specific state variables
  float wavePosition_;
  uint32_t lastFlickerUpdate_;
  uint32_t lightningStartTime_;
  bool lightningActive_;
  uint32_t lastSparkleUpdate_;
  uint8_t* sparklePixels_;  // Array to track sparkle states
};

}  // namespace espmods::led