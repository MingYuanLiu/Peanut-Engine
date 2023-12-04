#pragma once

#include "runtime/core/log/peanut_log.h"

#define PEANUT_LOG_DEBUG(...) \
  ::peanut::LogSytem::GetLogger()->debug(__VA_ARGS__)

#define PEANUT_LOG_INFO(...) ::peanut::LogSytem::GetLogger()->info(__VA_ARGS__)

#define PEANUT_LOG_WARN(...) ::peanut::LogSytem::GetLogger()->warn(__VA_ARGS__)

#define PEANUT_LOG_ERROR(...) \
  ::peanut::LogSytem::GetLogger()->error(__VA_ARGS__)

#define PEANUT_LOG_FATAL(...) \
  ::peanut::LogSytem::GetLogger()->critical(__VA_ARGS__)