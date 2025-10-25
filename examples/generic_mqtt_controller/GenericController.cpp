#include "GenericController.h"
#include <espmods/core.hpp>

using espmods::core::LogSerial;

GenericController::GenericController() {
  // Constructor
}

void GenericController::begin() {
  LogSerial.println("GenericController: Initialized");
}

void GenericController::loop() {
  // Main loop logic for the generic controller
  // This could be reading sensors, updating displays, etc.
}

void GenericController::handleMqttCommand(const String &topic, const String &payload) {
  LogSerial.println("GenericController: Received MQTT command on topic: " + topic + " with payload: " + payload);
  
  // Parse the topic to extract the command
  // Example topic format: "device/room/command"
  int lastSlash = topic.lastIndexOf('/');
  if (lastSlash >= 0) {
    String command = topic.substring(lastSlash + 1);
    processCommand(command, payload);
  } else {
    LogSerial.println("GenericController: Invalid topic format");
  }
}

void GenericController::processCommand(const String &command, const String &value) {
  if (command == "status") {
    LogSerial.println("GenericController: Status requested");
    // Could publish current status back via NetOtaMqtt
  } else if (command == "restart") {
    LogSerial.println("GenericController: Restart requested");
    // Could perform restart logic
  } else if (command == "config") {
    LogSerial.println("GenericController: Configuration change: " + value);
    // Could update configuration
  } else {
    LogSerial.println("GenericController: Unknown command: " + command);
  }
}