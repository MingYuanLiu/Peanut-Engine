#pragma once
#include <string>
#include "core/base/logger.h"

namespace peanut
{
	class StringUtils
	{
	public:

		template<typename ... Args >
		static std::string Format(const std::string& format_str, Args... args)
		{
			auto size_needed = std::snprintf(nullptr, 0, format_str.c_str(), args...) + 1; // Extra space for '\0'
			if (size_needed <= 0) 
			{ 
				PEANUT_LOG_ERROR("format error {0}", format_str);
				return "";
			}

			std::vector<char> buffer(size_needed);
			std::snprintf(buffer.data(), size_needed, format_str.c_str(), args...);
			return std::string(buffer.data(), buffer.size()); 
		}
	};
}