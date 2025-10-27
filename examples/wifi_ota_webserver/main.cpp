#include <Arduino.h>
#include <espmods/network.hpp>
#include <espmods/core.hpp>

using espmods::network::NetWifiOta;
using espmods::network::NetworkConfig;
using espmods::network::WidgetDashboard;
using espmods::core::LogSerial;

// Global network module instance
NetWifiOta netModule;
WidgetDashboard dashboard;

bool pulseEnabled = false;
float ledBrightness = 50.0f;
String latestMessage = "Ready";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  LogSerial.begin(Serial, "WiFiOtaExample");
  
  LogSerial.println("Starting WiFi OTA Web Server Example");
  LogSerial.println("This example demonstrates:");
  LogSerial.println("- WiFi connectivity management");
  LogSerial.println("- Over-The-Air (OTA) firmware updates");
  LogSerial.println("- Web server with live log streaming");
  
  // Create dashboard widgets
  WidgetDashboard::ButtonConfig pulseButton;
  pulseButton.id = "pulse";
  pulseButton.label = "Toggle Pulse";
  pulseButton.description = "Toggle the simulated output pulse";
  pulseButton.onClick = []() {
    pulseEnabled = !pulseEnabled;
    LogSerial.print("Pulse toggled: ");
    LogSerial.println(pulseEnabled ? "ON" : "OFF");
  };
  dashboard.addButton(pulseButton);

  WidgetDashboard::SliderConfig brightnessSlider;
  brightnessSlider.id = "brightness";
  brightnessSlider.label = "LED Brightness";
  brightnessSlider.min = 0.0f;
  brightnessSlider.max = 100.0f;
  brightnessSlider.step = 5.0f;
  brightnessSlider.value = ledBrightness;
  brightnessSlider.onChange = [](float value) {
    ledBrightness = value;
    LogSerial.print("Brightness slider → ");
    LogSerial.print(value, 1);
    LogSerial.println("%");
  };
  dashboard.addSlider(brightnessSlider);

  WidgetDashboard::InputConfig messageInput;
  messageInput.id = "message";
  messageInput.label = "Log Message";
  messageInput.placeholder = "Type a message";
  messageInput.value = latestMessage;
  messageInput.onSubmit = [](const String& value) {
    latestMessage = value;
    LogSerial.print("Dashboard message updated: ");
    LogSerial.println(latestMessage);
  };
  dashboard.addInput(messageInput);

  // Create network configuration
  NetworkConfig config;
  config.wifiSsid = "YourWiFiSSID";        // Replace with your WiFi SSID
  config.wifiPassword = "YourWiFiPassword"; // Replace with your WiFi password
  config.deviceHostname = "wifi-ota-example";
  config.webServerPort = 80;
  config.dashboard = &dashboard;
  
  // Initialize the network module
  netModule.begin(config);
  
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

    LogSerial.print(", Pulse: ");
    LogSerial.print(pulseEnabled ? "ON" : "OFF");
    LogSerial.print(", Brightness: ");
    LogSerial.print(ledBrightness, 1);
    LogSerial.print("%, Last message: ");
    LogSerial.print(latestMessage);
    LogSerial.println();

    lastLogMessage = now;
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}