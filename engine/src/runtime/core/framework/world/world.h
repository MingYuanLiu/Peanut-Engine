#pragma once
#include <string>

#include "runtime/core/reflection/marcos.h"
#include "runtime/core/framework/level/level.h"
#include "_generated_/reflection/all_reflection.h"

namespace peanut
{
	class META_CLASS() World
	{
		GENERATE_BODY(World);

	public:

		META_FUNCTION()
		std::string GetWorldName() { return std::string(); }

		META_FUNCTION()
		Level* GetCurrentLevel() { return nullptr; }

	private:
		META_PROPERTY()
		std::string world_name_;

		META_PROPERTY()
		Level* current_level_;
	};
}