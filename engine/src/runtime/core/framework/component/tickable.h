#pragma once

namespace peanut
{
	class ITickable 
	{
		virtual void Tick(float delta_time) = 0;
	};
}