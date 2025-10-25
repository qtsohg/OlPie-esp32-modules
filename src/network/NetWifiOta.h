#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <WiFi.h>
#include "NetworkConfig.h"

namespace espmods::network {

/**
 * @brief Network module that provides WiFi connectivity, OTA updates, and web server
 * 
 * This module handles:
 * - WiFi connection management
 * - Over-The-Air (OTA) firmware updates
 * - Web server with LogSerial output streaming
 * 
 * Unlike NetOtaMqtt, this module does not include MQTT functionality.
 */
class NetWifiOta {
 public:
  NetWifiOta();
  
  /**
   * @brief Initialize WiFi, OTA, and web server with configuration
   * @param config Network configuration containing WiFi credentials, hostname, etc.
   */
  void begin(const NetworkConfig& config);
  
  /**
   * @brief Main loop function - call this regularly from your main loop
   */
  void loop();
  
  /**
   * @brief Check if WiFi is connected
   * @return true if connected to WiFi, false otherwise
   */
  bool isWifiConnected() const;
  
  /**
   * @brief Get the current WiFi IP address
   * @return IPAddress of the device, or INADDR_NONE if not connected
   */
  IPAddress getIpAddress() const;
  
  /**
   * @brief Get WiFi signal strength
   * @return RSSI value in dBm
   */
  int32_t getWifiRssi() const;

 private:
  NetworkConfig config_;
  WebServer server_{80};
  bool webServerStarted_ = false;
  
  void setupWifi();
  void setupOta();
  void ensureWebServer();
  void handleWebServer();
};
}