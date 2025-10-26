#pragma once

#include <Arduino.h>

namespace espmods::audio {
// DY-SV5W playback states
enum class PlayState : uint8_t {
  STOP = 0x00,
  PLAY = 0x01,
  PAUSE = 0x02,
  UNKNOWN = 0xFF
};

class AudioDySv5w {
 public:
  explicit AudioDySv5w(uint8_t txPin, uint8_t rxPin);
  void begin(uint32_t baud = 9600);
  void play(uint16_t trackNumber);
  void sendCommand(uint8_t cmd, uint16_t param = 0);  // Generic command sender
  void endPlay();
  
  // Status checking
  PlayState getPlayState();  // Query and return current state from module
  bool isPlaying();  // Query module and return true if playing
  bool isBusy();  // Query module and return true if playing or paused
  void waitForPlayback(uint32_t timeoutMs = 5000);  // Block until playback stops
  
  void update();
  uint32_t lastPlayMillis() const { return lastPlayMillis_; }

 private:
  HardwareSerial serial_;
  uint8_t rxPin_;
  uint8_t txPin_;
  PlayState lastKnownState_ = PlayState::UNKNOWN;
  uint32_t lastPlayMillis_ = 0;
  
  // Helper to read response with timeout
  bool readResponse(uint8_t* buffer, size_t length, uint32_t timeoutMs = 100);
};

}