#pragma once

#include <memory>

#include "runtime/functions/rhi/rhi.h"
#include "runtime/functions/rhi/vulkan/vulkan_rhi.h"
#include "runtime/core/event/mouse_event.h"
#include "runtime/core/event/key_event.h"

#include "runtime/functions/render/render_pass_base.h"
#include "runtime/functions/render/render_pass.h"

namespace peanut {
class RenderSystem {
public:
	enum class InputMode
	{
		None,
		RotatingView,
		RotatingScene,
	};

 public:
  RenderSystem();
  ~RenderSystem() {}
  void Initialize(const std::shared_ptr<WindowSystem>& window_system);
  void Shutdown();
  void Tick();

  std::shared_ptr<RHI> GetRHI() { return rhi_; };
protected:
	void InitViewSettingAndSceneSetting();

	bool HandleMousePositionEvent(MouseMovedEvent& e);
	bool HandleMouseButtonEvent(MouseButtonEvent& e);
	bool HandleMouseScrollEvent(MouseScrolledEvent& e);
	bool HandleKeyEvent(KeyEvent& e);
private:
  std::shared_ptr<RHI> rhi_;
  std::unique_ptr<RenderPassBase> main_render_pass_;
  std::weak_ptr<WindowSystem> window_system_;

  // todo: register window event 
  ViewSettings view_;
  SceneSettings scene_;
  InputMode input_mode_;

  double last_mouse_pos_x_;
  double last_mouse_pos_y_;

  // TODO: viewport default config
  static constexpr float kViewDistance = 150.0f;
  static constexpr float kViewFOV = 45.0f;
  static constexpr float kOrbitSpeed = 1.0f;
  static constexpr float kZoomSpeed = 4.0f;
};
}  // namespace peanut