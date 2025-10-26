#include "AudioDySv5w.h"
#include <espmods/core.hpp>

using espmods::core::LogSerial;

namespace espmods::audio {
AudioDySv5w::AudioDySv5w(uint8_t txPin, uint8_t rxPin)
    : serial_(2), rxPin_(rxPin), txPin_(txPin) {  // use UART2
  // Pins stored, will be set in begin()
}

void AudioDySv5w::begin(uint32_t baud) {
  // CRITICAL: Must pass pins to begin(), not use setPins() before!
  // SERIAL_8N1 = 8 data bits, No parity, 1 stop bit (matches DY-SV5W spec)
  serial_.begin(baud, SERIAL_8N1, rxPin_, txPin_);  // RX, TX
  LogSerial.println("[AUDIO] DY-SV5W initialized on UART2");
  LogSerial.print("[AUDIO] Baud rate: ");
  LogSerial.println(baud);
  LogSerial.println("[AUDIO] TX Pin: 23 (ESP32 -> DY-SV5W RX)");
  LogSerial.println("[AUDIO] RX Pin: 34 (ESP32 <- DY-SV5W TX)");
  LogSerial.println("[AUDIO] Waiting 500ms for module to stabilize...");
  delay(500);
  LogSerial.println("[AUDIO] Ready - Querying module status...");
  getPlayState();
}

static uint8_t computeChecksum(const uint8_t *frame, size_t length) {
  // CRC: Sum of all bytes from start code through data, select low 8 bits
  // Example: AA 01 00 -> CRC = (AA + 01 + 00) & 0xFF = AB
  uint8_t sum = 0;
  for (size_t i = 0; i < length; ++i) {
    sum += frame[i];
  }
  return sum & 0xFF;
}

void AudioDySv5w::play(uint16_t trackNumber) {
  // Select specified music: AA 07 02 High_Byte Low_Byte CRC
  uint8_t frame[6];
  frame[0] = 0xAA;  // Start code
  frame[1] = 0x07;  // CMD: Select specified music
  frame[2] = 0x02;  // Data length: 2 bytes
  frame[3] = static_cast<uint8_t>((trackNumber >> 8) & 0xFF);  // High byte
  frame[4] = static_cast<uint8_t>(trackNumber & 0xFF);          // Low byte
  frame[5] = computeChecksum(frame, 5);  // CRC
  
  LogSerial.print("[AUDIO] Playing track #");
  LogSerial.print(trackNumber);
  LogSerial.print(" - Sending: ");
  for (size_t i = 0; i < 6; i++) {
    serial_.write(frame[i]);
    if (frame[i] < 0x10) LogSerial.print("0");
    LogSerial.print(frame[i], HEX);
    LogSerial.print(" ");
  }
  LogSerial.println();

  lastPlayMillis_ = millis();
}

void AudioDySv5w::playByFilename(const char* filename) {
  // Play file in root directory: /FILENAME*MP3
  // Format: AA 08 length device path CRC
  
  // Construct path: /FILENAME*MP3
  String path = "";
  path += filename;
  path += "*MP3";
  
  // Convert to uppercase as required by the spec
  path.toUpperCase();
  
  LogSerial.print("[AUDIO] Playing file by name: ");
  LogSerial.println(path);
  
  // Calculate frame size: start(1) + cmd(1) + length(1) + device(1) + path + crc(1)
  uint8_t pathLen = path.length();
  uint8_t totalDataLen = 1 + pathLen;  // device(1) + path length
  uint8_t frameSize = 4 + pathLen;     // AA + 08 + length + device + path + CRC
  
  uint8_t* frame = new uint8_t[frameSize];
  
  frame[0] = 0xAA;           // Start code
  frame[1] = 0x08;           // CMD: Specified device and path play
  frame[2] = totalDataLen;   // Length of device + path
  frame[3] = 0x00;           // Device: 0x00 for SD card/internal storage
  
  // Copy path bytes
  for (uint8_t i = 0; i < pathLen; i++) {
    frame[4 + i] = path[i];
  }
  
  // Calculate and add checksum
  frame[frameSize - 1] = computeChecksum(frame, frameSize - 1);
  
  // Send the frame
  LogSerial.print("[AUDIO] Sending filename command: ");
  for (size_t i = 0; i < frameSize; i++) {
    serial_.write(frame[i]);
    if (frame[i] < 0x10) LogSerial.print("0");
    LogSerial.print(frame[i], HEX);
    LogSerial.print(" ");
  }
  LogSerial.println();
  
  delete[] frame;
  lastPlayMillis_ = millis();
}

void AudioDySv5w::playByPath(const char* folder, const char* filename) {
  // Play file in specific folder: /FOLDER*/FILENAME*MP3
  // Format: AA 08 length device path CRC
  
  // Construct path: /FOLDER*/FILENAME*MP3
  String path = "/";
  if (folder[0] != '\0') {
    path += folder;
    path += "*/";
  }
  path += filename;
  path += "*MP3";
  
  // Convert to uppercase as required by the spec
  path.toUpperCase();
  
  LogSerial.print("[AUDIO] Playing file by path: ");
  LogSerial.println(path);
  
  // Calculate frame size: start(1) + cmd(1) + length(1) + device(1) + path + crc(1)
  uint8_t pathLen = path.length();
  uint8_t totalDataLen = 1 + pathLen;  // device(1) + path length
  uint8_t frameSize = 4 + pathLen;     // AA + 08 + length + device + path + CRC
  
  uint8_t* frame = new uint8_t[frameSize];
  
  frame[0] = 0xAA;           // Start code
  frame[1] = 0x08;           // CMD: Specified device and path play
  frame[2] = totalDataLen;   // Length of device + path
  frame[3] = 0x00;           // Device: 0x00 for SD card/internal storage
  
  // Copy path bytes
  for (uint8_t i = 0; i < pathLen; i++) {
    frame[4 + i] = path[i];
  }
  
  // Calculate and add checksum
  frame[frameSize - 1] = computeChecksum(frame, frameSize - 1);
  
  // Send the frame
  LogSerial.print("[AUDIO] Sending path command: ");
  for (size_t i = 0; i < frameSize; i++) {
    serial_.write(frame[i]);
    if (frame[i] < 0x10) LogSerial.print("0");
    LogSerial.print(frame[i], HEX);
    LogSerial.print(" ");
  }
  LogSerial.println();
  
  delete[] frame;
  lastPlayMillis_ = millis();
}

void AudioDySv5w::sendCommand(uint8_t cmd, uint16_t param) {
  // Generic command: AA CMD 02 High_Byte Low_Byte CRC
  uint8_t frame[6];
  frame[0] = 0xAA;  // Start code
  frame[1] = cmd;   // Command
  frame[2] = 0x02;  // Data length: 2 bytes
  frame[3] = static_cast<uint8_t>((param >> 8) & 0xFF);  // High byte
  frame[4] = static_cast<uint8_t>(param & 0xFF);          // Low byte
  frame[5] = computeChecksum(frame, 5);  // CRC
  
  LogSerial.print("[AUDIO] Sending CMD 0x");
  if (cmd < 0x10) LogSerial.print("0");
  LogSerial.print(cmd, HEX);
  LogSerial.print(" param=");
  LogSerial.print(param);
  LogSerial.print(": ");
  for (size_t i = 0; i < 6; i++) {
    serial_.write(frame[i]);
    if (frame[i] < 0x10) LogSerial.print("0");
    LogSerial.print(frame[i], HEX);
    LogSerial.print(" ");
  }
  LogSerial.println();

  delay(10);
  
  //serial_.write(frame, 6);
}

// End play function, like a force stop
//CMD：AA 10 00 BA
//Description：It can end the current playing，it will end and return to play original
//music, if the current operation is interlude.
void AudioDySv5w::endPlay() {
  // Generic command: AA CMD 02 High_Byte Low_Byte CRC
  uint8_t frame[6];
  frame[0] = 0xAA;  // Start code
  frame[1] = 0x10;   // Command
  frame[2] = 0x00;  // Data length: 2 bytes
  frame[3] = 0xBA;  // High byte

  LogSerial.print("[AUDIO] Sending CMD AA 10 00 BA");

  LogSerial.print(": ");
  for (size_t i = 0; i < 4; i++) {
    serial_.write(frame[i]);
    if (frame[i] < 0x10) LogSerial.print("0");
    LogSerial.print(frame[i], HEX);
    LogSerial.print(" ");
  }
  LogSerial.println();
}

bool AudioDySv5w::readResponse(uint8_t* buffer, size_t length, uint32_t timeoutMs) {
  uint32_t startTime = millis();
  size_t bytesRead = 0;
  
  while (bytesRead < length && (millis() - startTime < timeoutMs)) {
    if (serial_.available()) {
      buffer[bytesRead] = serial_.read();
      bytesRead++;
    }
    delay(1);  // Small delay to allow bytes to arrive
  }
  
  return bytesRead == length;
}

PlayState AudioDySv5w::getPlayState() {
  // Send query: AA 01 00 AB
  uint8_t query[4];
  query[0] = 0xAA;
  query[1] = 0x01;  // CMD: Check Play State
  query[2] = 0x00;  // Data length: 0
  query[3] = computeChecksum(query, 3);
  
  // Clear any old data
  while (serial_.available()) {
    serial_.read();
  }
  
  // Send query
  serial_.write(query, 4);
  
  // Wait for response: AA 01 01 State CRC
  uint8_t response[5];
  if (readResponse(response, 5, 200)) {
    // Validate response
    if (response[0] == 0xAA && response[1] == 0x01 && response[2] == 0x01) {
      uint8_t state = response[3];
      uint8_t expectedCrc = computeChecksum(response, 4);
      
      if (response[4] == expectedCrc) {
        lastKnownState_ = static_cast<PlayState>(state);
        
        LogSerial.print("[AUDIO] Play state: ");
        switch (lastKnownState_) {
          case PlayState::STOP:  LogSerial.println("STOP"); break;
          case PlayState::PLAY:  LogSerial.println("PLAY"); break;
          case PlayState::PAUSE: LogSerial.println("PAUSE"); break;
          default: LogSerial.println("UNKNOWN"); break;
        }
        
        return lastKnownState_;
      } else {
        LogSerial.println("[AUDIO] Invalid CRC in response");
      }
    } else {
      LogSerial.println("[AUDIO] Invalid response format");
    }
  } else {
    LogSerial.println("[AUDIO] No response from module");
  }
  
  lastKnownState_ = PlayState::UNKNOWN;
  return PlayState::UNKNOWN;
}

bool AudioDySv5w::isPlaying() {
  PlayState state = getPlayState();
  return state == PlayState::PLAY;
}

bool AudioDySv5w::isBusy() {
  PlayState state = getPlayState();
  return state == PlayState::PLAY || state == PlayState::PAUSE;
}

void AudioDySv5w::waitForPlayback(uint32_t timeoutMs) {
  LogSerial.print("[AUDIO] Waiting for playback to finish (timeout: ");
  LogSerial.print(timeoutMs);
  LogSerial.println(" ms)...");
  
  uint32_t startWait = millis();
  PlayState state;
  
  do {
    delay(100);  // Check every 100ms
    state = getPlayState();
    
    if (millis() - startWait >= timeoutMs) {
      LogSerial.println("[AUDIO] Wait timed out");
      return;
    }
  } while (state == PlayState::PLAY || state == PlayState::PAUSE);
  
  LogSerial.print("[AUDIO] Playback finished after ");
  LogSerial.print(millis() - startWait);
  LogSerial.println(" ms");
}

void AudioDySv5w::update() {
  // Optional: Can be used to log unexpected serial data
  if (serial_.available()) {
    LogSerial.print("[AUDIO] Unexpected serial data: ");
    while (serial_.available()) {
      uint8_t byte = serial_.read();
      if (byte < 0x10) LogSerial.print("0");
      LogSerial.print(byte, HEX);
      LogSerial.print(" ");
    }
    LogSerial.println();
  }
}
}