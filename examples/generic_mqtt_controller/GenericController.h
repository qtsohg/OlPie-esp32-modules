#pragma once

#include <Arduino.h>
#include <espmods/network.hpp>

using espmods::network::IMqttCommandHandler;

/**
 * @brief Example generic controller that can work with NetOtaMqtt
 * 
 * This demonstrates how any module can implement the IMqttCommandHandler
 * interface to work with the NetOtaMqtt network module.
 */
class GenericController : public IMqttCommandHandler {
 public:
  GenericController();
  
  void begin();
  void loop();
  
  // Implement the IMqttCommandHandler interface
  void handleMqttCommand(const String &topic, const String &payload) override;
  
 private:
  void processCommand(const String &command, const String &value);
};