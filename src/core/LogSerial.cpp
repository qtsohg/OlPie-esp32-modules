#include "LogSerial.h"

#include <cstring>

namespace espmods::core {

MirrorLog::MirrorLog(HardwareSerial &serial)
    : serial_(serial), head_(0), count_(0) {
  memset(buffer_, 0, sizeof(buffer_));
}

void MirrorLog::begin(unsigned long baudrate,
                      uint32_t config,
                      int8_t rxPin,
                      int8_t txPin,
                      bool invert,
                      unsigned long timeout_ms) {
  serial_.begin(baudrate, config, rxPin, txPin, invert, timeout_ms);
}

size_t MirrorLog::write(uint8_t ch) {
  size_t written = serial_.write(ch);
  buffer_[head_] = static_cast<char>(ch);
  head_ = (head_ + 1) % kBufferSize;
  if (count_ < kBufferSize) {
    ++count_;
  }
  return written;
}

size_t MirrorLog::write(const uint8_t *data, size_t size) {
  size_t written = serial_.write(data, size);
  for (size_t i = 0; i < size; ++i) {
    buffer_[head_] = static_cast<char>(data[i]);
    head_ = (head_ + 1) % kBufferSize;
    if (count_ < kBufferSize) {
      ++count_;
    }
  }
  return written;
}

// Helper to prepend timestamp
static String getTimestamp() {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  ms %= 1000;
  seconds %= 60;
  minutes %= 60;
  
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "[%02lu:%02lu:%02lu.%03lu] ", hours, minutes, seconds, ms);
  return String(buffer);
}

size_t MirrorLog::println(const String &s) {
  String msg = getTimestamp() + s;
  return Print::println(msg);
}

size_t MirrorLog::println(const char str[]) {
  return println(String(str));
}

size_t MirrorLog::println(char c) {
  return println(String(c));
}

size_t MirrorLog::println(unsigned char c, int base) {
  return println(String(c, base));
}

size_t MirrorLog::println(int num, int base) {
  return println(String(num, base));
}

size_t MirrorLog::println(unsigned int num, int base) {
  return println(String(num, base));
}

size_t MirrorLog::println(long num, int base) {
  return println(String(num, base));
}

size_t MirrorLog::println(unsigned long num, int base) {
  return println(String(num, base));
}

size_t MirrorLog::println(double num, int digits) {
  return println(String(num, digits));
}

size_t MirrorLog::println(void) {
  String msg = getTimestamp();
  return Print::println(msg);
}

int MirrorLog::available() const {
  return static_cast<int>(count_);
}

void MirrorLog::copyTo(String &out) const {
  out.reserve(out.length() + count_);
  size_t start = (head_ + kBufferSize - count_) % kBufferSize;
  for (size_t i = 0; i < count_; ++i) {
    size_t idx = (start + i) % kBufferSize;
    out += buffer_[idx];
  }
}

MirrorLog LogSerial(Serial);

}