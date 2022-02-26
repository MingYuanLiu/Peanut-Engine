#pragma once
#include "Core.h"

namespace Peanut_Engine {
	class PE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	};

	Application* CreateApplication();
}

