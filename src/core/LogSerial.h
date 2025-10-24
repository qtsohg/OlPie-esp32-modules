#pragma once

#include <Arduino.h>

namespace espmods::core {

// Call LogSerial.println(...) to mirror output to both the serial console and
// the web log buffer that backs the web console at /logs.
class MirrorLog : public Print {
 public:
  static constexpr size_t kBufferSize = 8192;

  explicit MirrorLog(HardwareSerial &serial);

  void begin(unsigned long baudrate,
             uint32_t config = SERIAL_8N1,
             int8_t rxPin = -1,
             int8_t txPin = -1,
             bool invert = false,
             unsigned long timeout_ms = 0);

  size_t write(uint8_t ch) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  
  // Override println to add timestamps
  size_t println(const String &s);
  size_t println(const char str[]);
  size_t println(char c);
  size_t println(unsigned char c, int base = DEC);
  size_t println(int num, int base = DEC);
  size_t println(unsigned int num, int base = DEC);
  size_t println(long num, int base = DEC);
  size_t println(unsigned long num, int base = DEC);
  size_t println(double num, int digits = 2);
  size_t println(void);

  int available() const;
  void copyTo(String &out) const;

  using Print::write;

 private:
  HardwareSerial &serial_;
  char buffer_[kBufferSize];
  size_t head_;
  size_t count_;
};

extern MirrorLog LogSerial;
}