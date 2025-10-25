#pragma once

#include <Arduino.h>
namespace espmods::network {
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
}