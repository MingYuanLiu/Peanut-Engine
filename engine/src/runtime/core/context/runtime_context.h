#pragma once

#include <memory>
#include <string>

#include "runtime/functions/window/window.h"
#include "runtime/functions/window/window_system.h"
#include "runtime/functions/render/render_system.h"
#include "runtime/core/log/peanut_log.h"

namespace peanut {
class RenderSystem;
/**
 * @brief manage the global states of the game engine runtime
 *
 */
class GlobalEngineContext 
{
public:
	static GlobalEngineContext* GetContext() 
	{
		static GlobalEngineContext global_context;
		return &global_context;
	}

	void SetupSubSystems();
	void DestroySubSystems();

	std::shared_ptr<RenderSystem> GetRenderSystem() { return render_system_; }
	std::shared_ptr<WindowSystem> GetWindowSystem() { return window_system_; }

public:
	std::shared_ptr<WindowSystem> window_system_;
	std::shared_ptr<RenderSystem> render_system_;

private:
	GlobalEngineContext() = default;
	~GlobalEngineContext() = default;

private:
	// TODO: use config system to config this values
	const std::string default_log_path_ = "./logs/runtime_log.txt";
	const uint32_t default_window_width_ = 1280;
	const uint32_t default_window_height_ = 720;
	const std::string default_window_title_ = "Peanut Engine";
};

}  // namespace peanut