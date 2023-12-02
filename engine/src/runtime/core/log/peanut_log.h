#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace Peanut {
class LogSystem {
 public:
  static bool init(const std::string& log_path, bool out_console = true);
  static void deinit();

  inline static std::shared_ptr<spdlog::logger> GetLogger() {
    return m_global_logger;
  }

 private:
  static std::shared_ptr<spdlog::logger> m_global_logger;
};
}  // namespace Peanut