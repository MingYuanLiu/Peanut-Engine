#pragma once

namespace peanut
{
	class WorldManager
	{
	public:
		void Initialize();
		void Clear();

		void Tick();

	private:
		bool LoadWorld();
	};
}