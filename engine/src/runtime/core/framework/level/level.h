#pragma once
#include <string>

#include "runtime/core/reflection/marcos.h"
#include "_generated_/reflection/all_reflection.h"

namespace peanut
{
	class META_CLASS() Level
	{
		GENERATE_BODY(Level);

	public:

		META_FUNCTION()
		std::string GetLevelName() { return level_name_; }
		
	private:
		META_PROPERTY()
		std::string level_name_;
	};
}