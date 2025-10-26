/*
 * LedStrip Usage Example
 * 
 * This example demonstrates how to use the LedStrip class for various effects.
 * Perfect for Halloween displays, ambient lighting, and other visual effects.
 */

#include <Arduino.h>
#include <espmods/led.hpp>

using espmods::led::LedStrip;
using espmods::led::LedEffect;
using espmods::led::LedEffectConfig;

// Create LED strip instance
// Pin 2, 50 LEDs, brightness 128
LedStrip ledStrip(2, 50, 128);

void setup() {
  Serial.begin(115200);
  Serial.println("LedStrip Example Starting...");
  
  // Initialize the LED strip
  ledStrip.begin();
  
  // Example 1: Simple solid color
  ledStrip.setSolidColor(0xFF0000); // Red
  delay(2000);
  
  // Example 2: Pulsing orange (great for Halloween)
  ledStrip.pulseColor(0xFF4400, 2000); // Orange pulse every 2 seconds
  delay(5000);
  
  // Example 3: Lightning effect
  ledStrip.lightning(0xFFFFFF); // White lightning
  delay(10000);
  
  // Example 4: Fire effect
  ledStrip.fire(200); // Intense fire
  delay(5000);
  
  // Example 5: Custom gradient pulse
  ledStrip.gradientPulse(0xFF0000, 0x000000, 3000); // Red to black
  delay(5000);
  
  // Example 6: Strobe for scare effect
  ledStrip.strobe(0xFF0000, 100); // Fast red strobe
  delay(3000);
  
  // Example 7: Sparkles
  ledStrip.sparkle(0xFFFFFF, 20); // White sparkles
  delay(5000);
  
  // Example 8: Using custom configuration
  LedEffectConfig config;
  config.effect = LedEffect::RandomFlicker;
  config.primaryColor = 0xFF4400; // Orange
  config.intensity = 100;
  config.speed = 50;
  ledStrip.setEffect(config);
  
  Serial.println("Setup complete - effects will cycle automatically in loop()");
}

void loop() {
  // Update the LED effects - MUST be called regularly
  ledStrip.update();
  
  // Example: Cycle through different Halloween effects
  static uint32_t lastEffectChange = 0;
  static uint8_t currentEffect = 0;
  
  if (millis() - lastEffectChange > 10000) { // Change effect every 10 seconds
    lastEffectChange = millis();
    
    switch (currentEffect) {
      case 0:
        Serial.println("Effect: Fire");
        ledStrip.fire(150);
        break;
      case 1:
        Serial.println("Effect: Lightning");
        ledStrip.lightning();
        break;
      case 2:
        Serial.println("Effect: Pulse Red");
        ledStrip.pulseColor(0xFF0000, 2000);
        break;
      case 3:
        Serial.println("Effect: Flicker Orange");
        ledStrip.randomFlicker(0xFF4400, 80);
        break;
      case 4:
        Serial.println("Effect: Strobe White");
        ledStrip.strobe(0xFFFFFF, 200);
        break;
      case 5:
        Serial.println("Effect: Sparkle Blue");
        ledStrip.sparkle(0x0066FF, 15);
        break;
    }
    
    currentEffect = (currentEffect + 1) % 6;
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}