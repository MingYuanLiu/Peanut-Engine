#include "runtime/core/context/runtime_context.h"
#include <direct.h>

std::string CurrentWorkDir()
{
	char buff[256];
	_getcwd(buff, 256);
	std::string current_work_dir(buff);
	return current_work_dir;
}

namespace peanut {
	void GlobalEngineContext::SetupSubSystems()
	{
		LogSystem::init(default_log_path_);

		PEANUT_LOG_INFO("Setup all engine subsystems");

		WindowCreateInfo create_info(default_window_width_, default_window_height_,
			default_window_title_);

		PEANUT_LOG_INFO("Current Work directory: {0}", CurrentWorkDir());

		window_system_ = std::make_shared<WindowSystem>();
		window_system_->Initialize(create_info);

		render_system_ = std::make_shared<RenderSystem>();
		render_system_->Initialize(window_system_);
	}

	void GlobalEngineContext::DestroySubSystems()
	{
		window_system_->Shutdown();
		render_system_->Shutdown();
	}
}  // namespace peanut