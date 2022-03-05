#include "pepch.h"
#include "Application.h"
#include "Log.h"
#include "Peanut/Event/KeyEvent.h"

namespace Peanut_Engine
{

	Application::Application() {

	}

	Application::~Application() {
		
	}

	void Application::Run() {

		KeyPressedEvent e(Key::A, 10);
		if (e.GetEventCategoryFlags() & EventCategoryKeyboard) {
			PE_INFO(e);
		}
		else {
			PE_CORE_WARN("Event category is not match.");
		}


		PE_CORE_WARN("Application Running ...");

		while (true) {

		}
	}
}