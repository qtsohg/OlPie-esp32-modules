#include <Arduino.h>
#include <espmods/network.hpp>
#include <espmods/core.hpp>
#include "GenericController.h"

using espmods::network::NetOtaMqtt;
using espmods::core::LogSerial;

// Global instances
NetOtaMqtt netModule;
GenericController controller;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  LogSerial.begin(Serial, "GenericExample");
  
  // Initialize the controller
  controller.begin();
  
  // Connect the controller to the network module
  netModule.setController(&controller);
  
  // Initialize the network module (WiFi, OTA, MQTT)
  netModule.begin();
  
  LogSerial.println("Generic MQTT Controller Example started");
}

void loop() {
  // Run the network module loop (handles WiFi, OTA, MQTT)
  netModule.loop();
  
  // Run the controller loop
  controller.loop();
  
  // Small delay to prevent watchdog issues
  delay(10);
}