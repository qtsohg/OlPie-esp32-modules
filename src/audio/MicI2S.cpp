#include "MicI2S.h"
#include <espmods/core.hpp>
#include <math.h>

using espmods::core::LogSerial;

namespace espmods::audio {
namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
constexpr float kI2sNormalization = 8388608.0f;  // 2^23 for 24-bit samples
constexpr float kSampleRate = 16000.0f;
constexpr bool kUseHannWindow = true;
constexpr float kGoertzelFrequencies[] = {210.0f, 240.0f, 270.0f, 480.0f, 120.0f, 390.0f};
constexpr size_t kGoertzelBinCount = sizeof(kGoertzelFrequencies) / sizeof(kGoertzelFrequencies[0]);
constexpr float kRatioEmaAlpha = 0.2f;
constexpr float kTonalityEmaAlpha = 0.2f;
constexpr float kDetectionEpsilon = 1e-6f;

struct GoertzelState {
  float coeff;
  float s1;
  float s2;
};

float hannWindow(size_t index, size_t total) {
  if (!kUseHannWindow || total <= 1) {
    return 1.0f;
  }
  const float phase = static_cast<float>(index) / static_cast<float>(total - 1);
  return 0.5f * (1.0f - cosf(kTwoPi * phase));
}
}  // namespace

MicI2S::MicI2S(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t data)
    : bclk_(bclk), lrclk_(lrclk), data_(data) {}

void MicI2S::begin() {
  i2s_config_t i2s_config = {
      .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = static_cast<int>(kSampleRate),
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = static_cast<int>(kSampleCount),
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0,
  };
  i2s_pin_config_t pin_config = {
      .bck_io_num = bclk_,
      .ws_io_num = lrclk_,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = data_,
  };
  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, nullptr);
  i2s_set_pin(I2S_NUM_1, &pin_config);
  i2s_set_clk(I2S_NUM_1, static_cast<uint32_t>(kSampleRate), I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO);
}

float MicI2S::sampleRms() { return rms_; }

float MicI2S::sampleWindowedRms() { return windowedRms_; }

void MicI2S::accumulateWindowSample(float sample) {
  if (windowFill_ < kWindowSize) {
    ++windowFill_;
  } else {
    float oldSample = window_[windowIndex_];
    windowSum_ -= oldSample;
    windowSumSquares_ -= static_cast<double>(oldSample) * oldSample;
  }
  window_[windowIndex_] = sample;
  windowSum_ += sample;
  windowSumSquares_ += static_cast<double>(sample) * sample;
  windowIndex_ = (windowIndex_ + 1) % kWindowSize;
}

void MicI2S::updateWindowedMetrics() {
  if (windowFill_ == 0) {
    return;
  }
  double count = static_cast<double>(windowFill_);
  double mean = windowSum_ / count;
  double meanSquares = windowSumSquares_ / count;
  double variance = meanSquares - (mean * mean);
  if (variance < 0.0) {
    variance = 0.0;
  }
  float rawRms = sqrtf(static_cast<float>(variance));
  if (!windowInitialized_) {
    windowedRms_ = rawRms;
    windowInitialized_ = true;
  } else {
    windowedRms_ = windowedRms_ * (1.0f - kWindowEmaAlpha) + rawRms * kWindowEmaAlpha;
  }
  if (windowedRms_ > 0.0f) {
    float dbfs = 20.0f * log10f(windowedRms_);
    windowedDbfs_ = dbfs > kDbfsFloor ? dbfs : kDbfsFloor;
  } else {
    windowedDbfs_ = kDbfsFloor;
  }
}

void MicI2S::setDetectionParams(const MicDetectionParams &params) {
  params_ = params;
  if (params_.debounceFrames == 0) {
    params_.debounceFrames = 1;
  }
}

void MicI2S::muteUntil(uint32_t millisUntil) { mutedUntil_ = millisUntil; }

bool MicI2S::fillFrame() {
  bool receivedSamples = false;
  while (frameFill_ < kSampleCount) {
    int32_t buffer[64];
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_NUM_1, buffer, sizeof(buffer), &bytesRead, 0);
    if (err != ESP_OK || bytesRead == 0) {
      break;
    }
    size_t samples = bytesRead / sizeof(int32_t);
    if (samples == 0) {
      break;
    }
    receivedSamples = true;
    for (size_t i = 0; i < samples && frameFill_ < kSampleCount; ++i) {
      int32_t raw24 = buffer[i] >> 8;  // 24-bit left justified
      float sample = static_cast<float>(raw24) / kI2sNormalization;
      frameBuffer_[frameFill_++] = sample;
      accumulateWindowSample(sample);
    }
    if (frameFill_ >= kSampleCount) {
      break;
    }
  }
  if (receivedSamples) {
    updateWindowedMetrics();
  }
  return frameFill_ >= kSampleCount;
}

void MicI2S::runGoertzel() {
  GoertzelState states[kGoertzelBinCount];
  for (size_t i = 0; i < kGoertzelBinCount; ++i) {
    const float omega = kTwoPi * kGoertzelFrequencies[i] / kSampleRate;
    states[i].coeff = 2.0f * cosf(omega);
    states[i].s1 = 0.0f;
    states[i].s2 = 0.0f;
  }

  double sumSquares = 0.0;
  for (size_t n = 0; n < kSampleCount; ++n) {
    float sample = frameBuffer_[n];
    sumSquares += static_cast<double>(sample) * sample;
    float window = hannWindow(n, kSampleCount);
    float windowedSample = sample * window;
    for (auto &state : states) {
      float s = windowedSample + state.coeff * state.s1 - state.s2;
      state.s2 = state.s1;
      state.s1 = s;
    }
  }

  for (size_t i = 0; i < kGoertzelBinCount; ++i) {
    const auto &state = states[i];
    float power = state.s1 * state.s1 + state.s2 * state.s2 - state.coeff * state.s1 * state.s2;
    if (power < 0.0f) {
      power = 0.0f;
    }
    detection_.bins[i] = power;
  }

  float harmonicSum = detection_.bins[0] + detection_.bins[1] + detection_.bins[2];
  float maxHarmonic = detection_.bins[0];
  if (detection_.bins[1] > maxHarmonic) {
    maxHarmonic = detection_.bins[1];
  }
  if (detection_.bins[2] > maxHarmonic) {
    maxHarmonic = detection_.bins[2];
  }
  float numerator = harmonicSum + 0.5f * detection_.bins[3];
  float denominator = detection_.bins[4] + detection_.bins[5] + kDetectionEpsilon;
  float ratio = numerator / denominator;
  float tonality = maxHarmonic / (harmonicSum + kDetectionEpsilon);

  if (!ratioInitialized_) {
    ratioEma_ = ratio;
    ratioInitialized_ = true;
  } else {
    ratioEma_ = ratioEma_ + kRatioEmaAlpha * (ratio - ratioEma_);
  }
  if (!tonalityInitialized_) {
    tonalityEma_ = tonality;
    tonalityInitialized_ = true;
  } else {
    tonalityEma_ = tonalityEma_ + kTonalityEmaAlpha * (tonality - tonalityEma_);
  }

  detection_.ratio = ratio;
  detection_.ratioEma = ratioEma_;
  detection_.tonality = tonality;
  detection_.tonalityEma = tonalityEma_;
  detection_.rms = sqrtf(static_cast<float>(sumSquares / static_cast<double>(kSampleCount)));
  detection_.frameValid = true;
  rms_ = detection_.rms;
}

MicDetectionResult MicI2S::update(float ratioOnThreshold, float ratioHoldThreshold) {
  if (ratioOnThreshold > 0.0f) {
    params_.ratioOn = ratioOnThreshold;
  }
  if (ratioHoldThreshold > 0.0f) {
    params_.ratioHold = ratioHoldThreshold;
  }

  uint32_t now = millis();
  bool haveFrame = fillFrame();
  if (haveFrame) {
    runGoertzel();
    frameFill_ = 0;
  } else {
    detection_.frameValid = false;
  }

  if (now < mutedUntil_) {
    active_ = false;
    onStreak_ = 0;
    offStreak_ = 0;
    detection_.brushing = false;
    return detection_;
  }

  if (haveFrame) {
    bool passesOn = (ratioEma_ >= params_.ratioOn) && (tonalityEma_ >= params_.tonalityOn);
    bool passesHold = (ratioEma_ >= params_.ratioHold) && (tonalityEma_ >= params_.tonalityHold);

    if (!active_) {
      if (passesOn) {
        if (onStreak_ < params_.debounceFrames) {
          ++onStreak_;
        }
        if (onStreak_ >= params_.debounceFrames) {
          active_ = true;
          offStreak_ = 0;
        }
      } else {
        onStreak_ = 0;
      }
    } else {
      if (passesHold) {
        offStreak_ = 0;
      } else {
        if (offStreak_ < params_.debounceFrames) {
          ++offStreak_;
        }
        if (offStreak_ >= params_.debounceFrames) {
          active_ = false;
          onStreak_ = 0;
        }
      }
    }

    static uint32_t lastLog = 0;
    if (now - lastLog >= 500000) {
      lastLog = now;
      LogSerial.println("Mic ratio (inst/EMA): " + String(detection_.ratio, 3) + "/" +
                        String(detection_.ratioEma, 3));
      LogSerial.println("Mic tonality (inst/EMA): " + String(detection_.tonality, 3) + "/" +
                        String(detection_.tonalityEma, 3));
      LogSerial.println("Ratio thresholds ON/HOLD: " + String(params_.ratioOn, 2) + "/" +
                        String(params_.ratioHold, 2));
      LogSerial.println("Tonality thresholds ON/HOLD: " + String(params_.tonalityOn, 2) + "/" +
                        String(params_.tonalityHold, 2));
    }
  }

  detection_.brushing = active_;
  return detection_;
}

bool MicI2S::brushingActive() const {
  if (millis() < mutedUntil_) {
    return false;
  }
  return active_;
}
}