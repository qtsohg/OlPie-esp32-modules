#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>

#include "config.h"
#include "secrets.h"

namespace espmods::network {

class IMqttCommandHandler;

class NetOtaMqtt {
 public:
  NetOtaMqtt();
  void setController(IMqttCommandHandler *controller);
  void begin();
  void loop();
  void publishState(const String &state);
  void publishElapsed(uint32_t seconds);
  void publishStreak(uint32_t streak);
  void publishProfile(const String &profileName);
  void publishThresholdOn(float threshold);
  void publishThresholdOff(float threshold);
  void publishMicRms(float rms);
  void publishMicDbfs(float dbfs);
  void publishMicActive(bool active);
  void publishAudioTrack(uint16_t track);
  void publishAudioPlaying(bool playing);
  void publishDiscovery();
  void publishInitialStates();

 private:
  void ensureWebServer();
  void handleWebServer();

  WiFiClient wifiClient_;
  PubSubClient mqttClient_;
  WebServer server_{80};
  IMqttCommandHandler *controller_ = nullptr;
  uint32_t lastMqttAttempt_ = 0;
  bool webServerStarted_ = false;

  void setupWifi();
  void setupOta();
  void ensureMqtt();
  void onMqttMessage(char *topic, uint8_t *payload, unsigned int length);
};
}