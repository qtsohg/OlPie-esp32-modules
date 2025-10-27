#include "NetWifiOta.h"
#include "NetworkConfig.h"
#include "WidgetDashboard.h"

#include <espmods/core.hpp>

using espmods::core::LogSerial;

namespace espmods::network {

NetWifiOta::NetWifiOta() {}

void NetWifiOta::setupWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  
  LogSerial.println("Setting up WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(config_.wifiSsid, config_.wifiPassword);
  
  // Wait for connection (with timeout)
  uint32_t startTime = millis();
  const uint32_t timeout = 15000; // 15 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    LogSerial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    LogSerial.println();
    LogSerial.println("WiFi connected!");
    LogSerial.print("IP Address: ");
    LogSerial.println(WiFi.localIP());
    LogSerial.print("RSSI: ");
    LogSerial.print(WiFi.RSSI());
    LogSerial.println(" dBm");
  } else {
    LogSerial.println();
    LogSerial.println("WiFi connection failed!");
  }
}

void NetWifiOta::setupOta() {
  static bool otaStarted = false;
  if (otaStarted) return;
  
  LogSerial.println("Setting up OTA...");
  ArduinoOTA.setHostname(config_.deviceHostname);
  
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    LogSerial.println("OTA Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    LogSerial.println("OTA Update complete");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static uint8_t lastPercent = 255;
    uint8_t percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      LogSerial.print("OTA Progress: ");
      LogSerial.print(percent);
      LogSerial.println("%");
      lastPercent = percent;
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    LogSerial.print("OTA Error: ");
    switch (error) {
      case OTA_AUTH_ERROR: LogSerial.println("Auth Failed"); break;
      case OTA_BEGIN_ERROR: LogSerial.println("Begin Failed"); break;
      case OTA_CONNECT_ERROR: LogSerial.println("Connect Failed"); break;
      case OTA_RECEIVE_ERROR: LogSerial.println("Receive Failed"); break;
      case OTA_END_ERROR: LogSerial.println("End Failed"); break;
      default: LogSerial.println("Unknown Error"); break;
    }
  });
  
  ArduinoOTA.begin();
  LogSerial.println("OTA ready");
  otaStarted = true;
}

void NetWifiOta::begin(const NetworkConfig& config) {
  config_ = config;
  
  // Update server port if specified
  if (config_.webServerPort != 80) {
    server_.~WebServer();
    new (&server_) WebServer(config_.webServerPort);
  }
  
  setupWifi();
  setupOta();
  LogSerial.println("NetWifiOta initialization complete");
}

void NetWifiOta::ensureWebServer() {
  if (webServerStarted_ || WiFi.status() != WL_CONNECTED) {
    return;
  }

  LogSerial.println("Starting web server...");

  if (config_.dashboard) {
    config_.dashboard->attach(server_, config_);
  } else {
    server_.on("/", [this]() {
      static const char kIndexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Device Logs</title>
  <style>
    body {
      font-family: 'Courier New', monospace;
      background: #1a1a1a;
      color: #e0e0e0;
      margin: 0;
      padding: 0;
      line-height: 1.4;
    }
    header {
      padding: 1rem;
      background: #2d2d2d;
      position: sticky;
      top: 0;
      border-bottom: 2px solid #444;
      box-shadow: 0 2px 4px rgba(0,0,0,0.3);
    }
    h1 {
      margin: 0;
      font-size: 1.4rem;
      color: #4CAF50;
    }
    .status {
      font-size: 0.9rem;
      color: #888;
      margin-top: 0.5rem;
    }
    .controls {
      margin-top: 1rem;
    }
    button {
      background: #4CAF50;
      color: white;
      border: none;
      padding: 8px 16px;
      margin-right: 8px;
      border-radius: 4px;
      cursor: pointer;
      font-family: inherit;
    }
    button:hover {
      background: #45a049;
    }
    button:disabled {
      background: #666;
      cursor: not-allowed;
    }
    #logs {
      padding: 1rem;
      height: calc(100vh - 140px);
      overflow-y: auto;
      white-space: pre-wrap;
      font-size: 0.9rem;
      background: #0d1117;
      border: 1px solid #333;
      margin: 1rem;
      border-radius: 4px;
    }
    .auto-scroll {
      background: #ff9800 !important;
    }
    .auto-scroll:hover {
      background: #f57c00 !important;
    }
  </style>
</head>
<body>
  <header>
    <h1>Device Logs</h1>
    <div class="status" id="status">Loading...</div>
    <div class="controls">
      <button onclick="clearLogs()">Clear Display</button>
      <button onclick="downloadLogs()">Download Logs</button>
      <button id="autoScrollBtn" onclick="toggleAutoScroll()" class="auto-scroll">Auto-scroll: ON</button>
    </div>
  </header>
  <pre id="logs">Loading logs...</pre>

  <script>
    const logsEl = document.getElementById('logs');
    const statusEl = document.getElementById('status');
    const autoScrollBtn = document.getElementById('autoScrollBtn');
    let autoScroll = true;
    let lastUpdateTime = new Date();

    function updateStatus() {
      const now = new Date();
      const timeDiff = Math.round((now - lastUpdateTime) / 1000);
      statusEl.textContent = `Last update: ${timeDiff}s ago | Auto-refresh: ON`;
    }

    async function fetchLogs() {
      try {
        const response = await fetch('/logs', { cache: 'no-cache' });
        if (!response.ok) {
          throw new Error(`HTTP ${response.status}`);
        }

        const shouldPin = autoScroll &&
          Math.abs(logsEl.scrollTop + logsEl.clientHeight - logsEl.scrollHeight) < 8;

        const newContent = await response.text();
        logsEl.textContent = newContent;

        if (shouldPin) {
          logsEl.scrollTop = logsEl.scrollHeight;
        }

        lastUpdateTime = new Date();
        updateStatus();
      } catch (err) {
        console.error('Failed to fetch logs:', err);
        statusEl.textContent = `Error: ${err.message}`;
      }
    }

    function clearLogs() {
      logsEl.textContent = 'Logs cleared (this only clears the display, not the actual logs)';
    }

    function downloadLogs() {
      const logContent = logsEl.textContent;
      const blob = new Blob([logContent], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `device-logs-${new Date().toISOString().slice(0,19).replace(/:/g,'-')}.txt`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }

    function toggleAutoScroll() {
      autoScroll = !autoScroll;
      autoScrollBtn.textContent = `Auto-scroll: ${autoScroll ? 'ON' : 'OFF'}`;
      autoScrollBtn.className = autoScroll ? 'auto-scroll' : '';
    }

    setInterval(updateStatus, 1000);
    setInterval(fetchLogs, 1000);
    fetchLogs();
  </script>
</body>
</html>
)rawliteral";
      server_.send_P(200, "text/html", kIndexHtml);
    });
  }

  // API endpoint to get logs
  server_.on("/logs", [this]() {
    String logs;
    LogSerial.copyTo(logs);
    server_.send(200, "text/plain", logs);
  });

  // Device info endpoint
  server_.on("/info", [this]() {
    String info = "{\n";
    info += "  \"hostname\": \"" + String(config_.deviceHostname) + "\",\n";
    info += "  \"ip\": \"" + WiFi.localIP().toString() + "\",\n";
    info += "  \"rssi\": " + String(WiFi.RSSI()) + ",\n";
    info += "  \"mac\": \"" + WiFi.macAddress() + "\",\n";
    info += "  \"uptime\": " + String(millis()) + ",\n";
    info += "  \"free_heap\": " + String(ESP.getFreeHeap()) + ",\n";
    info += "  \"chip_model\": \"" + String(ESP.getChipModel()) + "\",\n";
    info += "  \"chip_revision\": " + String(ESP.getChipRevision()) + ",\n";
    info += "  \"sdk_version\": \"" + String(ESP.getSdkVersion()) + "\"\n";
    info += "}";
    server_.send(200, "application/json", info);
  });

  // 404 handler
  server_.onNotFound([this]() {
    server_.send(404, "text/plain", "Not Found");
  });

  server_.begin();
  webServerStarted_ = true;
  LogSerial.print("Web server started on port ");
  LogSerial.println(config_.webServerPort);
  LogSerial.print("Access logs at: http://");
  LogSerial.println(WiFi.localIP());
}

void NetWifiOta::handleWebServer() {
  if (webServerStarted_) {
    server_.handleClient();
  }
}

void NetWifiOta::loop() {
  // Ensure WiFi stays connected
  if (WiFi.status() != WL_CONNECTED) {
    static uint32_t lastReconnectAttempt = 0;
    uint32_t now = millis();
    
    // Try to reconnect every 30 seconds
    if (now - lastReconnectAttempt > 30000) {
      LogSerial.println("WiFi disconnected, attempting to reconnect...");
      setupWifi();
      lastReconnectAttempt = now;
    }
  } else {
    // WiFi is connected, ensure web server is running
    ensureWebServer();
    handleWebServer();
  }
  
  // Handle OTA updates
  ArduinoOTA.handle();
}

bool NetWifiOta::isWifiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress NetWifiOta::getIpAddress() const {
  return WiFi.localIP();
}

int32_t NetWifiOta::getWifiRssi() const {
  return WiFi.RSSI();
}

}