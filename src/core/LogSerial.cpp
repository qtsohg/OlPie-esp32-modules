#include "LogSerial.h"

#include <cstring>
#include <cstdarg>
#include "esp_log.h"

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

size_t MirrorLog::printf(const char* format, ...) {
  char buffer[512];  // Adjust size as needed
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  if (len > 0) {
    String msg = getTimestamp() + String(buffer);
    return Print::print(msg);
  }
  return 0;
}

size_t MirrorLog::printfln(const char* format, ...) {
  char buffer[512];  // Adjust size as needed
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  if (len > 0) {
    String msg = getTimestamp() + String(buffer);
    return Print::println(msg);
  }
  return 0;
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

// Serial capture implementation using ESP-IDF logging redirection
static bool g_capture_enabled = false;

// Override the ESP-IDF log output function
static int log_vprintf(const char* format, va_list args) {
  if (g_capture_enabled) {
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    if (len > 0) {
      // Add system prefix and timestamp through LogSerial
      String msg = String("[SYS] ") + String(buffer);
      LogSerial.write(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length());
    }
  }
  // Also send to original output
  return vprintf(format, args);
}

void enableSerialCapture() {
  if (!g_capture_enabled) {
    g_capture_enabled = true;
    // Set our custom log function
    esp_log_set_vprintf(log_vprintf);
    LogSerial.printfln("Serial capture enabled - system logs will appear in web console");
  }
}

void disableSerialCapture() {
  if (g_capture_enabled) {
    g_capture_enabled = false;
    // Restore default log function
    esp_log_set_vprintf(vprintf);
    LogSerial.printfln("Serial capture disabled");
  }
}

// Advanced system capture
static bool g_full_capture_enabled = false;
static FILE* g_original_stdout = nullptr;
static FILE* g_original_stderr = nullptr;

// Custom write function for stdout/stderr redirection
static int custom_stdout_write(void* cookie, const char* data, int size) {
  // Write to LogSerial buffer
  if (g_full_capture_enabled && data && size > 0) {
    String msg = "[STDOUT] " + String(data).substring(0, size);
    LogSerial.write(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length());
  }
  // Also write to original stdout
  return fwrite(data, 1, size, g_original_stdout);
}

static int custom_stderr_write(void* cookie, const char* data, int size) {
  // Write to LogSerial buffer
  if (g_full_capture_enabled && data && size > 0) {
    String msg = "[STDERR] " + String(data).substring(0, size);
    LogSerial.write(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length());
  }
  // Also write to original stderr
  return fwrite(data, 1, size, g_original_stderr);
}

void enableFullSystemCapture() {
  if (!g_full_capture_enabled) {
    g_full_capture_enabled = true;
    
    // Enable ESP-IDF log capture
    enableSerialCapture();
    
    // Store original stdout/stderr
    g_original_stdout = stdout;
    g_original_stderr = stderr;
    
    LogSerial.printfln("Full system capture enabled - all output will appear in web console");
  }
}

void disableFullSystemCapture() {
  if (g_full_capture_enabled) {
    g_full_capture_enabled = false;
    
    // Disable ESP-IDF log capture
    disableSerialCapture();
    
    // Restore original stdout/stderr if they were redirected
    if (g_original_stdout) {
      stdout = g_original_stdout;
      g_original_stdout = nullptr;
    }
    if (g_original_stderr) {
      stderr = g_original_stderr;
      g_original_stderr = nullptr;
    }
    
    LogSerial.printfln("Full system capture disabled");
  }
}

}  // namespace espmods::core

// Support for serial redirection macro
#ifdef ENABLE_SERIAL_REDIRECT
HardwareSerial* _original_serial = &Serial;
#endif