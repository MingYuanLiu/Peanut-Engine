#include "pepch.h"
#include "Application.h"
#include "Log.h"
#include "Peanut/Event/KeyEvent.h"

namespace Peanut_Engine
{

	Application::Application() {
		m_window_ = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {
		
	}

	void Application::Run() {
		while (m_running) {
			m_window_->Update();
		}
	}
}