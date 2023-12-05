#include "runtime/core/log/peanut_log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace peanut {
std::shared_ptr<spdlog::logger> LogSystem::m_global_logger_;

bool LogSystem::init(const std::string& log_path, bool out_console) {
  std::vector<spdlog::sink_ptr> log_sinks;

  if (out_console)
    log_sinks.emplace_back(
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  if (!log_path.empty())
    log_sinks.emplace_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, true));

  for (auto& sink : log_sinks) {
    sink->set_level(spdlog::level::trace);
    sink->set_pattern("%^[%Y-%m-%d-%H-%M-%S.%e][%P-%t][%l] %v%$");
  }

  spdlog::init_thread_pool(8192, 1);

  m_global_logger_ = std::make_shared<spdlog::async_logger>(
      "Peanut_Logger", log_sinks.begin(), log_sinks.end(),
      spdlog::thread_pool(), spdlog::async_overflow_policy::block);

  m_global_logger_->set_level(spdlog::level::trace);
  spdlog::register_logger(m_global_logger_);

  return true;
}

void LogSystem::deinit() {
  m_global_logger_->flush();
  spdlog::drop_all();
}
}  // namespace peanut