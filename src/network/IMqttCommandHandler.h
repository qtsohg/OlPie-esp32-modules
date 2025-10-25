#pragma once

#include <Arduino.h>

namespace espmods::network {
/**
 * @brief Interface for handling MQTT commands
 * 
 * This interface defines the contract for classes that can handle MQTT commands
 * from the NetOtaMqtt module. Any controller or module that needs to receive
 * MQTT commands should implement this interface.
 */
class IMqttCommandHandler {
 public:
  virtual ~IMqttCommandHandler() = default;
  
  /**
   * @brief Handle an MQTT command received from the network module
   * @param topic The MQTT topic the command was received on
   * @param payload The payload/message content of the command
   */
  virtual void handleMqttCommand(const String &topic, const String &payload) = 0;
};
}