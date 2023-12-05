#pragma once

#include <memory>
#include <string>

#include "runtime/functions/window/window.h"
#include "runtime/functions/window/window_system.h"

namespace peanut {

class LogSystem;
class WindowSystem;

/**
 * @brief manage the global states of the game engine runtime
 *
 */
class GlobalRuntimeContext {
 public:
  static GlobalRuntimeContext* GetContext() {
    static GlobalRuntimeContext global_context;
    return &global_context;
  }

  void SetupSubSystems() {
    WindowCreateInfo create_info(default_window_width_, default_window_height_,
                                 default_window_title_);
    window_system_ = std::make_shared<WindowSystem>(create_info);
  }
  void DestorySubSystems() { window_system_->Shutdown(); }

 public:
  std::shared_ptr<WindowSystem> window_system_;

 private:
  GlobalRuntimeContext() = default;
  ~GlobalRuntimeContext() = default;

 private:
  // TODO: use config system to config this values
  const std::string default_log_path_ = "./logs/runtime_log.txt";
  const uint32_t default_window_width_ = 1280;
  const uint32_t default_window_height_ = 720;
  const std::string default_window_title_ = "Peanut";
};

}  // namespace peanut