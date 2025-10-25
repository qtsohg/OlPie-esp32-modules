#include <Arduino.h>
#include <espmods/network.hpp>
#include <espmods/core.hpp>

using espmods::network::NetWifiOta;
using espmods::core::LogSerial;

// Global network module instance
NetWifiOta netModule;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  LogSerial.begin(Serial, "WiFiOtaExample");
  
  LogSerial.println("Starting WiFi OTA Web Server Example");
  LogSerial.println("This example demonstrates:");
  LogSerial.println("- WiFi connectivity management");
  LogSerial.println("- Over-The-Air (OTA) firmware updates");
  LogSerial.println("- Web server with live log streaming");
  
  // Initialize the network module
  netModule.begin();
  
  LogSerial.println("Setup complete!");
  if (netModule.isWifiConnected()) {
    LogSerial.print("Access web interface at: http://");
    LogSerial.println(netModule.getIpAddress());
  }
}

void loop() {
  // Run the network module (handles WiFi, OTA, and web server)
  netModule.loop();
  
  // Your application logic here
  static uint32_t lastLogMessage = 0;
  uint32_t now = millis();
  
  // Log a periodic message to demonstrate the web log viewer
  if (now - lastLogMessage > 10000) { // Every 10 seconds
    LogSerial.print("Periodic status update - Uptime: ");
    LogSerial.print(now / 1000);
    LogSerial.print("s, Free heap: ");
    LogSerial.print(ESP.getFreeHeap());
    LogSerial.print(" bytes");
    
    if (netModule.isWifiConnected()) {
      LogSerial.print(", WiFi RSSI: ");
      LogSerial.print(netModule.getWifiRssi());
      LogSerial.print(" dBm");
    } else {
      LogSerial.print(", WiFi: Disconnected");
    }
    LogSerial.println();
    
    lastLogMessage = now;
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}