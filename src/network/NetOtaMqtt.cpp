#include "NetOtaMqtt.h"

#include <ArduinoJson.h>
#include <functional>

#include <espmods/core.hpp>
#include <espmods/network.hpp>

using espmods::core::LogSerial;
using espmods::network::IMqttCommandHandler;

namespace espmods::network {

NetOtaMqtt::NetOtaMqtt() : mqttClient_(wifiClient_) {}

void NetOtaMqtt::setController(IMqttCommandHandler *controller) {
  controller_ = controller;
}

void NetOtaMqtt::setupWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void NetOtaMqtt::setupOta() {
  static bool otaStarted = false;
  if (otaStarted) return;
  ArduinoOTA.setHostname(DEVICE_HOSTNAME);
  ArduinoOTA.begin();
  otaStarted = true;
}

void NetOtaMqtt::begin() {
  setupWifi();
  setupOta();
  mqttClient_.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient_.setBufferSize(1024);  // Increase buffer size for discovery messages
  mqttClient_.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
    onMqttMessage(topic, payload, length);
  });

  // Print the IP address for debugging
  LogSerial.println("Wifi Setup complete");
  LogSerial.print("IP Address: ");
  LogSerial.println(WiFi.localIP());
}

void NetOtaMqtt::ensureWebServer() {
  if (webServerStarted_ || WiFi.status() != WL_CONNECTED) {
    return;
  }

  server_.on(
      "/", [this]() {
        static const char kIndexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Toothbrush Logs</title>
  <style>
    body { font-family: monospace; background: #111; color: #eee; margin: 0; padding: 0; }
    header { padding: 1rem; background: #222; position: sticky; top: 0; }
    h1 { margin: 0; font-size: 1.2rem; }
    #logs { padding: 1rem; height: calc(100vh - 3.5rem); overflow-y: auto; white-space: pre-wrap; }
  </style>
</head>
<body>
  <header><h1>Toothbrush Logs</h1></header>
  <pre id="logs"></pre>
  <script>
    const logsEl = document.getElementById('logs');
    async function fetchLogs() {
      try {
        const response = await fetch('/logs', { cache: 'no-cache' });
        if (!response.ok) return;
        const shouldPin = Math.abs(logsEl.scrollTop + logsEl.clientHeight - logsEl.scrollHeight) < 8;
        logsEl.textContent = await response.text();
        if (shouldPin) {
          logsEl.scrollTop = logsEl.scrollHeight;
        }
      } catch (err) {
        console.error(err);
      }
    }
    setInterval(fetchLogs, 100);
    fetchLogs();
  </script>
</body>
</html>
)rawliteral";
        server_.send_P(200, "text/html", kIndexHtml);
      });

  server_.on(
      "/logs", [this]() {
        String logs;
        LogSerial.copyTo(logs);
        // The browser UI polls /logs once a second to stream mirrored serial output.
        server_.send(200, "text/plain", logs);
      });

  server_.begin();
  webServerStarted_ = true;
}

void NetOtaMqtt::handleWebServer() {
  if (!webServerStarted_) {
    return;
  }
  server_.handleClient();
}

void NetOtaMqtt::publishDiscovery() {
  if (!mqttClient_.connected()) return;
  /// dbgln("Publishing MQTT Discovery");
  LogSerial.println("Publishing MQTT Discovery");
  struct DiscoveryEntry {
    const char *topic;
    const char *component;
    std::function<void(JsonDocument &)> fill;
  };
  DiscoveryEntry entries[] = {
      {TOPIC_CMND_START, "button", [this](JsonDocument &doc) {
         doc["name"] = "Start Brush";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_START);
       }},
      {TOPIC_CMND_PAUSE, "button", [this](JsonDocument &doc) {
         doc["name"] = "Pause Brush";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_PAUSE);
       }},
      {TOPIC_CMND_NEXT_PROFILE, "button", [this](JsonDocument &doc) {
         doc["name"] = "Next Profile";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_NEXT_PROFILE);
       }},
      {TOPIC_CMND_PLAY_TRACK, "number", [this](JsonDocument &doc) {
         doc["name"] = "Play Track";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_PLAY_TRACK);
         doc["min"] = 1;
         doc["max"] = 100;
         doc["step"] = 1;
         doc["icon"] = "mdi:play-circle";
       }},
      {TOPIC_PROFILE, "select", [this](JsonDocument &doc) {
         doc["name"] = "Profile";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_PROFILE);
         doc["state_topic"] = mqttTopic(TOPIC_PROFILE);
         JsonArray options = doc.createNestedArray("options");
         for (size_t i = 0; i < PROFILE_COUNT; ++i) {
           options.add(PROFILES[i].name);
         }
       }},
      {TOPIC_STATE, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Brush State";
         doc["state_topic"] = mqttTopic(TOPIC_STATE);
         doc["icon"] = "mdi:toothbrush";
       }},
      {TOPIC_ELAPSED, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Elapsed";
         doc["state_topic"] = mqttTopic(TOPIC_ELAPSED);
         doc["unit_of_measurement"] = "s";
       }},
      {TOPIC_STREAK, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Streak";
         doc["state_topic"] = mqttTopic(TOPIC_STREAK);
       }},
      {TOPIC_THRESHOLD_ON, "number", [this](JsonDocument &doc) {
         doc["name"] = "Mic Threshold ON";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_THRESHOLD_ON);
         doc["state_topic"] = mqttTopic(TOPIC_THRESHOLD_ON);
         doc["min"] = 0.001;
         doc["max"] = 0.03;
         doc["step"] = 0.001;
         doc["icon"] = "mdi:microphone";
       }},
      {TOPIC_THRESHOLD_OFF, "number", [this](JsonDocument &doc) {
         doc["name"] = "Mic Threshold OFF";
         doc["command_topic"] = mqttTopic(TOPIC_CMND_THRESHOLD_OFF);
         doc["state_topic"] = mqttTopic(TOPIC_THRESHOLD_OFF);
         doc["min"] = 0.001;
         doc["max"] = 0.03;
         doc["step"] = 0.001;
         doc["icon"] = "mdi:microphone";
       }},
      {TOPIC_MIC_RMS, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Mic RMS";
         doc["state_topic"] = mqttTopic(TOPIC_MIC_RMS);
         doc["unit_of_measurement"] = "";
         doc["state_class"] = "measurement";
         doc["icon"] = "mdi:waveform";
       }},
      {TOPIC_MIC_DBFS, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Mic dBFS";
         doc["state_topic"] = mqttTopic(TOPIC_MIC_DBFS);
         doc["unit_of_measurement"] = "dBFS";
         doc["state_class"] = "measurement";
         doc["icon"] = "mdi:decibel";
       }},
      {TOPIC_MIC_ACTIVE, "binary_sensor", [this](JsonDocument &doc) {
         doc["name"] = "Mic Active";
         doc["state_topic"] = mqttTopic(TOPIC_MIC_ACTIVE);
         doc["payload_on"] = "ON";
         doc["payload_off"] = "OFF";
         doc["icon"] = "mdi:microphone";
       }},
      {TOPIC_AUDIO_TRACK, "sensor", [this](JsonDocument &doc) {
         doc["name"] = "Audio Track";
         doc["state_topic"] = mqttTopic(TOPIC_AUDIO_TRACK);
         doc["icon"] = "mdi:music-note";
       }},
      {TOPIC_AUDIO_PLAYING, "binary_sensor", [this](JsonDocument &doc) {
         doc["name"] = "Audio Playing";
         doc["state_topic"] = mqttTopic(TOPIC_AUDIO_PLAYING);
         doc["payload_on"] = "ON";
         doc["payload_off"] = "OFF";
         doc["icon"] = "mdi:speaker";
       }},
  };
  for (const auto &entry : entries) {
    StaticJsonDocument<512> doc;
    entry.fill(doc);
    
    // Sanitize object_id - replace slashes with underscores
    String objectId = String(entry.topic);
    objectId.replace("/", "_");
    
    doc["unique_id"] = String(DEVICE_ID) + "_" + objectId;
    
    // Device information
    JsonObject device = doc.createNestedObject("device");
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(DEVICE_ID);
    device["name"] = "Toothbrush Timer";
    device["manufacturer"] = "Toothtron";
    device["model"] = "ESP32";
    device["sw_version"] = "1.0.0";
    
    // Home Assistant discovery topic format: <prefix>/<component>/<node_id>/<object_id>/config
    String configTopic = String(MQTT_DISCOVERY_PREFIX) + "/" + entry.component + "/" + DEVICE_ID + "/" + objectId + "/config";
    String payload;
    serializeJson(doc, payload);
    
    LogSerial.println("Publishing discovery to: " + configTopic);
    LogSerial.println("Payload length: " + String(payload.length()));
    LogSerial.println("Payload: " + payload);
    
    bool published = mqttClient_.publish(configTopic.c_str(), payload.c_str(), true);
    if (published) {
      LogSerial.println("  -> SUCCESS");
    } else {
      LogSerial.println("  -> FAILED!");
    }
  }
}

void NetOtaMqtt::publishState(const String &state) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_STATE).c_str(), state.c_str(), true);
}

void NetOtaMqtt::publishElapsed(uint32_t seconds) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_ELAPSED).c_str(), String(seconds).c_str(), true);
}

void NetOtaMqtt::publishStreak(uint32_t streak) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_STREAK).c_str(), String(streak).c_str(), true);
}

void NetOtaMqtt::publishProfile(const String &profileName) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_PROFILE).c_str(), profileName.c_str(), true);
}

void NetOtaMqtt::publishThresholdOn(float threshold) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_THRESHOLD_ON).c_str(), String(threshold, 3).c_str(), true);
}

void NetOtaMqtt::publishThresholdOff(float threshold) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_THRESHOLD_OFF).c_str(), String(threshold, 3).c_str(), true);
}

void NetOtaMqtt::publishMicRms(float rms) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_MIC_RMS).c_str(), String(rms, 4).c_str(), false);  // Not retained
}

void NetOtaMqtt::publishMicDbfs(float dbfs) {
  if (!mqttClient_.connected()) return;
  if (!(dbfs == dbfs)) {
    dbfs = -120.0f;
  }
  if (dbfs > 0.0f) {
    dbfs = 0.0f;
  }
  if (dbfs < -200.0f) {
    dbfs = -200.0f;
  }
  mqttClient_.publish(mqttTopic(TOPIC_MIC_DBFS).c_str(), String(dbfs, 1).c_str(), false);  // Not retained
}

void NetOtaMqtt::publishMicActive(bool active) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_MIC_ACTIVE).c_str(), active ? "ON" : "OFF", false);  // Not retained
}

void NetOtaMqtt::publishAudioTrack(uint16_t track) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_AUDIO_TRACK).c_str(), String(track).c_str(), false);  // Not retained
}

void NetOtaMqtt::publishAudioPlaying(bool playing) {
  if (!mqttClient_.connected()) return;
  mqttClient_.publish(mqttTopic(TOPIC_AUDIO_PLAYING).c_str(), playing ? "ON" : "OFF", false);  // Not retained
}

void NetOtaMqtt::publishInitialStates() {
  if (!mqttClient_.connected()) return;
  LogSerial.println("Publishing initial states...");
  /*
  // Publish initial values so Home Assistant recognizes the entities
  publishState("Idle");
  publishElapsed(0);
  publishStreak(0);
  publishProfile("Theodore");  // Default profile
  publishThresholdOn(1.80f);   // Default spectral ratio ON threshold
  publishThresholdOff(1.40f);  // Default spectral ratio HOLD threshold
  publishMicRms(0.0);          // Initial mic reading
  publishMicDbfs(-120.0f);     // Initial mic level
  publishMicActive(false);     // Initial mic state
  LogSerial.println("Initial states published");*/
}

void NetOtaMqtt::onMqttMessage(char *topic, uint8_t *payload, unsigned int length) {
  if (!controller_) return;
  String message;
  for (unsigned int i = 0; i < length; ++i) {
    message += static_cast<char>(payload[i]);
  }
  controller_->handleMqttCommand(String(topic), message);
}

void NetOtaMqtt::ensureMqtt() {
  if (mqttClient_.connected()) {
    mqttClient_.loop();
    return;
  }
  uint32_t now = millis();
  if (now - lastMqttAttempt_ < 5000) {
    return;
  }
  lastMqttAttempt_ = now;
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
    return;
  }
  if (mqttClient_.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) {
    LogSerial.println("MQTT Connected!");
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_START).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_PAUSE).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_NEXT_PROFILE).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_PROFILE).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_THRESHOLD_ON).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_THRESHOLD_OFF).c_str());
    mqttClient_.subscribe(mqttTopic(TOPIC_CMND_PLAY_TRACK).c_str());
    publishDiscovery();
    // Publish initial states so Home Assistant recognizes the entities
    publishInitialStates();
  }
}

void NetOtaMqtt::loop() {
  setupOta();
  ensureMqtt();
  ArduinoOTA.handle();
  ensureWebServer();
  handleWebServer();
}

}