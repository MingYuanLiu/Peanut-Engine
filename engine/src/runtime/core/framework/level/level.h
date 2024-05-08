#pragma once
#include <string>

#include "runtime/core/reflection/marcos.h"

namespace peanut
{
	class META_CLASS() Level
	{
	public:

		META_FUNCTION()
		std::string GetLevelName() { return level_name_; }
		
	private:
		META_PROPERTY()
		std::string level_name_;
	};
}