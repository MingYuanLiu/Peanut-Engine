#include "runtime/core/context/runtime_context.h"

namespace peanut {
	void GlobalRuntimeContext::SetupSubSystems()
	{
		LogSystem::init(default_log_path_);

		PEANUT_LOG_INFO("Setup all engine subsystems");

		WindowCreateInfo create_info(default_window_width_, default_window_height_,
			default_window_title_);

		window_system_ = std::make_shared<WindowSystem>();
		window_system_->Initialize(create_info);

		render_system_ = std::make_shared<RenderSystem>();
		render_system_->Initialize(window_system_);
	}

	void GlobalRuntimeContext::DestroySubSystems()
	{
		window_system_->Shutdown();
		render_system_->Shutdown();
	}
}  // namespace peanut