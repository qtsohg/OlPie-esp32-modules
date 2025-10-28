#pragma once

#include "espmods/core.hpp"

// Optional: Macro to redirect Serial calls to LogSerial
// Include this file AFTER Arduino.h if you want to capture library Serial output

#ifdef ENABLE_SERIAL_REDIRECT

// Save original Serial for internal use
extern HardwareSerial* _original_serial;

// Macro to redirect Serial.print/println calls to LogSerial
#define Serial LogSerial

// If libraries need access to the original serial
#define OriginalSerial (*_original_serial)

#endif // ENABLE_SERIAL_REDIRECT