#include <Arduino.h>
#include <espmods/core.hpp>

espmods::core::Module module;

void setup() {
  Serial.begin(115200);
  module.begin();
}

void loop() {
  module.update();
}
