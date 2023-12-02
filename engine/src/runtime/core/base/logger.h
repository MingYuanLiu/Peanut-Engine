#include "runtime/core/log/peanut_log.h"

#define PEANUT_LOG_DEBUG(...) \
  ::Peanut::LogSytem::GetLogger()->debug(__VA_ARGS__)

#define PEANUT_LOG_INFO(...) ::Peanut::LogSytem::GetLogger()->info(__VA_ARGS__)

#define PEANUT_LOG_WARN(...) ::Peanut::LogSytem::GetLogger()->warn(__VA_ARGS__)

#define PEANUT_LOG_ERROR(...) \
  ::Peanut::LogSytem::GetLogger()->error(__VA_ARGS__)

#define PEANUT_LOG_FATAL(...) \
  ::Peanut::LogSytem::GetLogger()->critical(__VA_ARGS__)