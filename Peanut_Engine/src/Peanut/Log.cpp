#include "Log.h"

namespace Peanut_Engine
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		spdlog::set_pattern("*** %^[%T]%s:%# %n: %v%$ ***");
		s_CoreLogger = spdlog::stdout_color_mt("PE_CORE");
		s_ClientLogger = spdlog::stdout_color_mt("PE_APP");
	}
}