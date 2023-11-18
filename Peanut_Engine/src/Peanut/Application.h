#pragma once
#include "Peanut/Core.h"
#include "Peanut/Window.h"
#include "Peanut/Event/ApplicationEvent.h"
#include "Peanut/Layer.h"
#include "Peanut/LayerStack.h"

namespace Peanut_Engine
{
	class PE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);
		void Run();
		bool OnWindowCloseEvent(WindowCloseEvent& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
	private:
		std::unique_ptr<Window> m_window_;
		bool m_running = true;

		LayerStack m_layerstack_;
	};

	Application* CreateApplication();
}

