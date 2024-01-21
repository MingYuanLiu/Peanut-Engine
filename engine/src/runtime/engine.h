#pragma once
#include "runtime/core/event/event.h"
#include "runtime/core/event/window_event.h"

namespace peanut {
	class PeanutEngine {
	public:
		static PeanutEngine& GetInstance() {
			static PeanutEngine peanut_engine;
			return peanut_engine;
		}
		void Initliaze();
		void Shutdown();
		void Run();

		bool HandleWindowCloseEvent(WindowCloseEvent& e);

		PeanutEngine(const PeanutEngine&) = delete;
		PeanutEngine& operator=(const PeanutEngine&) = delete;
	private:
		PeanutEngine() = default;
		~PeanutEngine() = default;

		bool IsShutdown = false;
	};
}