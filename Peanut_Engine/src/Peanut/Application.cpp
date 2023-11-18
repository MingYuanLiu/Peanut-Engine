#include "pepch.h"
#include "Application.h"
#include "Log.h"

namespace Peanut_Engine
{
#define PE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); } 

	Application::Application() {
		m_window_ = std::unique_ptr<Window>(Window::Create());
		m_window_->SetWindowEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

	Application::~Application() {
		
	}

	void Application::PushLayer(Layer* layer) {
		m_layerstack_.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay) {
		m_layerstack_.PushOverlay(overlay);
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(PE_BIND_EVENT_FN(Application::OnWindowCloseEvent));

		PE_CORE_INFO("{0}", e);

		for (auto riter = m_layerstack_.rbegin(); riter != m_layerstack_.rend(); riter++) {
			(*riter)->OnEvent(e);
			if (e.m_handled)
				break;
		}
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent& event) {
		m_running = false; 

		return true;
	}

	void Application::Run() {
		while (m_running) {
			m_window_->Update();

			for (auto iter = m_layerstack_.begin(); iter != m_layerstack_.end(); iter++) {
				(*iter)->OnUpdate();
			}
		}
	}
}