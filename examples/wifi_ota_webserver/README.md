# NetWifiOta Module

## Overview

The `NetWifiOta` module provides a lightweight network solution that handles WiFi connectivity, Over-The-Air (OTA) firmware updates, and a web server with live log streaming. Unlike `NetOtaMqtt`, this module does not include MQTT functionality, making it perfect for applications that only need basic networking features.

## Features

### 🌐 WiFi Management
- Automatic WiFi connection with configurable credentials
- Connection monitoring and automatic reconnection
- WiFi status reporting (IP address, RSSI, etc.)

### 🔄 Over-The-Air (OTA) Updates
- Secure firmware updates over WiFi
- Progress monitoring and error handling
- Arduino IDE and PlatformIO compatible

### 🖥️ Web Server with Live Logs
- Real-time log streaming from `LogSerial`
- Modern, responsive web interface
- Features include:
  - Auto-scrolling log viewer
  - Log download functionality
  - Device information endpoint
  - Dark theme optimized for readability

## Usage

### Basic Setup

```cpp
#include <espmods/network.hpp>
#include <espmods/core.hpp>

using espmods::network::NetWifiOta;
using espmods::core::LogSerial;

NetWifiOta netModule;

void setup() {
  Serial.begin(115200);
  LogSerial.begin(Serial, "MyDevice");
  
  // Initialize network module
  netModule.begin();
}

void loop() {
  // Handle WiFi, OTA, and web server
  netModule.loop();
  
  // Your application code here
}
```

### Configuration

The module uses the same configuration as other network modules:
- `WIFI_SSID` and `WIFI_PASS` from `secrets.h`
- `DEVICE_HOSTNAME` from `config.h`

### API Methods

```cpp
// Check WiFi connection status
bool isConnected = netModule.isWifiConnected();

// Get device IP address
IPAddress ip = netModule.getIpAddress();

// Get WiFi signal strength
int32_t rssi = netModule.getWifiRssi();
```

## Web Interface

Once connected to WiFi, access the web interface at:
```
http://[device-ip-address]/
```

### Available Endpoints

- `/` - Main log viewer interface
- `/logs` - Raw log data (text/plain)
- `/info` - Device information (JSON)

### Web Interface Features

1. **Live Log Streaming**: Automatically refreshes every second
2. **Auto-scroll**: Keeps the latest logs visible
3. **Download Logs**: Save current logs to a file
4. **Clear Display**: Clear the web view (doesn't affect actual logs)
5. **Device Status**: Shows connection info and last update time

## OTA Updates

### Arduino IDE
1. After the device connects to WiFi, it will appear in:
   - **Tools > Port** as a network port
2. Select the network port and upload as normal

### PlatformIO
Add to your `platformio.ini`:
```ini
upload_protocol = espota
upload_port = [device-ip-address]
```

## Comparison with NetOtaMqtt

| Feature | NetWifiOta | NetOtaMqtt |
|---------|------------|------------|
| WiFi Management | ✅ | ✅ |
| OTA Updates | ✅ | ✅ |
| Web Server | ✅ | ✅ |
| Live Log Streaming | ✅ | ✅ |
| MQTT Client | ❌ | ✅ |
| Controller Interface | ❌ | ✅ (IMqttCommandHandler) |
| Use Case | Simple logging/monitoring | IoT with remote control |

## Example Use Cases

- **Development/Debugging**: Live log monitoring during development
- **Field Deployment**: Remote firmware updates and status monitoring
- **Simple IoT Devices**: Devices that need connectivity but not MQTT
- **Standalone Sensors**: Log data viewing without external broker

## Memory Usage

NetWifiOta has a smaller memory footprint than NetOtaMqtt since it doesn't include:
- MQTT client libraries
- JSON message handling
- Command processing logic

This makes it ideal for memory-constrained applications or when you simply need basic network connectivity.