#pragma once
#include "runtime/core/reflection/marcos.h"

namespace peanut
{
	META_CLASS()
	class Level
	{
	public:

		META_FUNCTION()
		std::string GetLevelName();
		
	private:
		META_PROPERTY()
		std::string level_name_;
	};
}