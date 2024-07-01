#pragma once
#include <string>
#include "core/base/logger.h"

namespace peanut
{
	class ProcessUtils
	{
	public:

		static bool ExecProcess(const std::string& command, std::string& output)
		{

#ifdef __linux__
			FILE* pipe = popen(command.c_str(), "r");
#elif WIN32
			FILE* pipe = _popen(command.c_str(), "r");
#endif
			if (!pipe)
			{
				PEANUT_LOG_ERROR("Create pipe failed for command {0}", command);
				return false;
			}

			char output_buffer[128];
			while (fgets(output_buffer, sizeof(output_buffer), pipe))
			{
				output += output_buffer;
			}

#ifdef __linux__
			int ret_code = pclose(pipe);
#elif WIN32
			int ret_code = _pclose(pipe);
#endif
			if (ret_code != 0)
			{
				PEANUT_LOG_ERROR("Execute command {0} failed, return code {1}", command, ret_code);
				return false;
			}

			return true;
		}
	};
}