#pragma once

#include <Arduino.h>
#include <IPAddress.h>

namespace espmods {
namespace network {

/**
 * @brief Configuration structure for network modules
 */
struct NetworkConfig {
  // WiFi credentials
  const char* wifiSsid;
  const char* wifiPassword;
  
  // Device identification
  const char* deviceHostname;
  
  // MQTT settings (optional, used by NetOtaMqtt)
  const char* mqttHost = nullptr;
  uint16_t mqttPort = 1883;
  const char* mqttUser = nullptr;
  const char* mqttPassword = nullptr;
  
  // Web server port (optional, default 80)
  uint16_t webServerPort = 80;
};

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

}  // namespace network
}  // namespace espmods

#include "network/NetWifiOta.h"
