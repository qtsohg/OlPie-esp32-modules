#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

namespace espmods::audio {

struct MicDetectionParams {
  float ratioOn = 1.8f;         // EMA ratio threshold to declare brushing (overridden by profiles)
  float ratioHold = 1.4f;       // EMA ratio threshold to stay in brushing (overridden by profiles)
  float tonalityOn = 0.55f;     // EMA tonality threshold to declare brushing
  float tonalityHold = 0.45f;   // EMA tonality threshold to stay in brushing
  uint8_t debounceFrames = 5;   // Number of consecutive frames for state flips
};

struct MicDetectionResult {
  bool brushing = false;        // Current brushing state after debounce
  bool frameValid = false;      // True when the most recent Goertzel frame ran
  float rms = 0.0f;             // RMS of the latest frame
  float ratio = 0.0f;           // Instantaneous spectral ratio for this frame
  float ratioEma = 0.0f;        // Smoothed spectral ratio (EMA)
  float tonality = 0.0f;        // Instantaneous tonality for this frame
  float tonalityEma = 0.0f;     // Smoothed tonality (EMA)
  float bins[6] = {0.0f};       // Bin order: 210, 240, 270, 480, 120, 390 Hz
};

class MicI2S {
 public:
  MicI2S(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t data);
  void begin();
  float sampleRms();
  float sampleWindowedRms();
  float windowedRms() const { return windowedRms_; }
  float windowedDbfs() const { return windowedDbfs_; }
  MicDetectionResult update(float ratioOnThreshold, float ratioHoldThreshold);
  bool brushingActive() const;
  void muteUntil(uint32_t millisUntil);
  const MicDetectionResult &lastDetection() const { return detection_; }
  void setDetectionParams(const MicDetectionParams &params);
  const MicDetectionParams &detectionParams() const { return params_; }

 private:
  gpio_num_t bclk_;
  gpio_num_t lrclk_;
  gpio_num_t data_;
  static constexpr size_t kSampleCount = 512;
  static constexpr size_t kWindowSize = 1024;
  static constexpr float kWindowEmaAlpha = 0.2f;
  static constexpr float kDbfsFloor = -120.0f;
  float rms_ = 0.0f;
  float window_[kWindowSize] = {};
  size_t windowIndex_ = 0;
  size_t windowFill_ = 0;
  double windowSum_ = 0.0;
  double windowSumSquares_ = 0.0;
  float windowedRms_ = 0.0f;
  float windowedDbfs_ = kDbfsFloor;
  bool windowInitialized_ = false;
  bool active_ = false;
  uint32_t mutedUntil_ = 0;
  MicDetectionParams params_;
  MicDetectionResult detection_;
  bool ratioInitialized_ = false;
  bool tonalityInitialized_ = false;
  float ratioEma_ = 0.0f;
  float tonalityEma_ = 0.0f;
  uint8_t onStreak_ = 0;
  uint8_t offStreak_ = 0;
  float frameBuffer_[kSampleCount] = {};
  size_t frameFill_ = 0;
  void accumulateWindowSample(float sample);
  void updateWindowedMetrics();
  bool fillFrame();
  void runGoertzel();
};
}
