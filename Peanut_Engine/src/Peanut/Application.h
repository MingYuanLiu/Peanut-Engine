#pragma once
#include "Peanut/Core.h"
#include "Peanut/Window.h"

namespace Peanut_Engine
{
	class PE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		std::unique_ptr<Window> m_window_;
		bool m_running = true;
	};

	Application* CreateApplication();
}

