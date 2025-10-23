#pragma once

namespace espmods::core {

/**
 * @brief Minimal placeholder module you can extend with real functionality.
 */
class Module {
public:
  /**
   * @brief Initialize hardware or state needed by the module.
   */
  void begin();

  /**
   * @brief Periodic update hook. Call this from your loop.
   */
  void update();
};

}  // namespace espmods::core
