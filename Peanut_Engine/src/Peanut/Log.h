#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Peanut_Engine
{
	class PE_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}


// wapper for core logger
#define PE_CORE_TRACE(...)			::Peanut_Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PE_CORE_INFO(...)			::Peanut_Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PE_CORE_WARN(...)			::Peanut_Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PE_CORE_ERROR(...)			::Peanut_Engine::Log::GetCoreLogger()->error(__VA_ARGS__)


// wapper for client logger
#define PE_TRACE(...)				::Peanut_Engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PE_INFO(...)				::Peanut_Engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define PE_WARN(...)				::Peanut_Engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PE_ERROR(...)				::Peanut_Engine::Log::GetClientLogger()->error(__VA_ARGS__)
