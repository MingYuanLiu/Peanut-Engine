#pragma once

#include "runtime/core/log/peanut_log.h"

#define PEANUT_LOG_DEBUG(...) \
  ::peanut::LogSystem::GetLogger()->debug(__VA_ARGS__)

#define PEANUT_LOG_INFO(...) ::peanut::LogSystem::GetLogger()->info(__VA_ARGS__)

#define PEANUT_LOG_WARN(...) ::peanut::LogSystem::GetLogger()->warn(__VA_ARGS__)

#define PEANUT_LOG_ERROR(...) \
  ::peanut::LogSystem::GetLogger()->error(__VA_ARGS__)

#define PEANUT_LOG_FATAL(...) \
  ::peanut::LogSystem::GetLogger()->critical(__VA_ARGS__)