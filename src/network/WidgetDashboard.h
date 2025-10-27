#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include <functional>
#include <vector>

namespace espmods::network {

struct NetworkConfig;

class WidgetDashboard {
 public:
  using ButtonCallback = std::function<void()>;
  using SliderCallback = std::function<void(float)>;
  using InputCallback = std::function<void(const String&)>;

  struct ButtonConfig {
    String id;
    String label;
    String description;
    ButtonCallback onClick;
    String endpoint;
  };

  struct SliderConfig {
    String id;
    String label;
    float min = 0.0f;
    float max = 100.0f;
    float step = 1.0f;
    float value = 0.0f;
    SliderCallback onChange;
    String endpoint;
  };

  struct InputConfig {
    String id;
    String label;
    String placeholder;
    String value;
    InputCallback onSubmit;
    String endpoint;
  };

  void addButton(const ButtonConfig& button);
  void addSlider(const SliderConfig& slider);
  void addInput(const InputConfig& input);

  void attach(WebServer& server, const NetworkConfig& config);

 private:
  String buildIndexHtml(const NetworkConfig& config) const;

  std::vector<ButtonConfig> buttons_;
  std::vector<SliderConfig> sliders_;
  std::vector<InputConfig> inputs_;
};

}  // namespace espmods::network

