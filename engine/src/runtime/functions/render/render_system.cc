#include "runtime/functions/render/render_system.h"

#include "runtime/functions/render/render_utils.h"

namespace peanut {
RenderSystem::RenderSystem()
    : rhi_(std::make_shared<VulkanRHI>()),
      main_render_pass_(std::make_unique<MainRenderPass>()),
      last_mouse_pos_x_(0.0f),
      last_mouse_pos_y_(0.0f),
      input_mode_(InputMode::None) {}

void RenderSystem::Initialize(const std::shared_ptr<WindowSystem>& window_system) 
{
    PEANUT_LOG_INFO("Intialize Render system");
    rhi_->Init(window_system);
    main_render_pass_->Initialize();
    InitViewSettingAndSceneSetting();

    window_system->PushEventCallback([this](Event& e) 
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseButtonEvent>(
            BIND_EVENT_FN(RenderSystem::HandleMouseButtonEvent));
        dispatcher.Dispatch<MouseMovedEvent>(
            BIND_EVENT_FN(RenderSystem::HandleMousePositionEvent));
        dispatcher.Dispatch<MouseScrolledEvent>(
            BIND_EVENT_FN(RenderSystem::HandleMouseScrollEvent));
        dispatcher.Dispatch<KeyEvent>(BIND_EVENT_FN(RenderSystem::HandleKeyEvent));
    });

    window_system_ = window_system;
}

void RenderSystem::Shutdown() 
{
    rhi_->Shutdown();
    main_render_pass_->DeInitialize();
}

void RenderSystem::Tick() { main_render_pass_->RenderTick(view_, scene_); }

void RenderSystem::InitViewSettingAndSceneSetting()
{
    view_.distance = kViewDistance;
    view_.fov = kViewFOV;

    scene_.lights[0].direction = glm::normalize(glm::vec3{-1.0f, 0.0f, 0.0f});
    scene_.lights[1].direction = glm::normalize(glm::vec3{1.0f, 0.0f, 0.0f});
    scene_.lights[2].direction = glm::normalize(glm::vec3{0.0f, -1.0f, 0.0f});

    scene_.lights[0].radiance = glm::vec3{1.0f};
    scene_.lights[1].radiance = glm::vec3{1.0f};
    scene_.lights[2].radiance = glm::vec3{1.0f};
}

bool RenderSystem::HandleMousePositionEvent(MouseMovedEvent& e) 
{
    PEANUT_LOG_INFO("Handle mouse position");
    if (input_mode_ != InputMode::None) {
    const double delta_x = RenderUtils::CheckDoubleNearZero(last_mouse_pos_x_)
                                ? 0
                                : e.GetX() - last_mouse_pos_x_;
    const double delta_y = RenderUtils::CheckDoubleNearZero(last_mouse_pos_y_)
                                ? 0
                                : e.GetY() - last_mouse_pos_y_;

    switch (input_mode_) 
    {
        case InputMode::RotatingView:
        {
            view_.yaw += kOrbitSpeed * (float)(delta_x);
            view_.pitch += kOrbitSpeed * (float)(delta_y);
            break;
        }
        case InputMode::RotatingScene:
        {
            scene_.yaw += kOrbitSpeed * (float)(delta_x);
            scene_.pitch += kOrbitSpeed * (float)(delta_y);
            break;
        }
        default:
            break;
    }

    last_mouse_pos_x_ = e.GetX();
    last_mouse_pos_y_ = e.GetY();
    }

    return true;
}

bool RenderSystem::HandleMouseButtonEvent(MouseButtonEvent& e) {
  InputMode new_input_mode = InputMode::None;
  MouseCode code = e.GetMouseButton();
  PEANUT_LOG_INFO("Handle mouse button ({0}) change input mode", code);
  if (e.IsPressed() && input_mode_ == InputMode::None) {
    switch (code) {
      case Mouse::ButtonLeft:
        new_input_mode = InputMode::RotatingView;
        break;
      case Mouse::ButtonRight:
        new_input_mode = InputMode::RotatingScene;
        break;
      default:
        PEANUT_LOG_ERROR("No support this mouse code {0}", code);
        break;
    }
  }

  if (!e.IsPressed() &&
      (code == Mouse::ButtonLeft || code == Mouse::ButtonRight)) {
    new_input_mode = InputMode::None;
  }

  if (new_input_mode != input_mode_) {
    auto window_system = window_system_.lock();
    if (new_input_mode == InputMode::None) {
      window_system->SetMouseCapture(false);
    } else {
      window_system->SetMouseCapture(true);
    }
  }

  input_mode_ = new_input_mode;

  return true;
}

bool RenderSystem::HandleMouseScrollEvent(MouseScrolledEvent& e) {
  view_.distance += kZoomSpeed * float(-e.GetYOffset());
  return true;
}

bool RenderSystem::HandleKeyEvent(KeyEvent& e) {
  KeyCode code = e.GetKeyCode();
  PEANUT_LOG_INFO("Get key event ({0}) in render system, change light config",
                  code);

  if (e.IsPressed()) {
    SceneSettings::Light* light = nullptr;

    switch (code) {
      case Key::U:
        light = &scene_.lights[0];
        break;
      case Key::I:
        light = &scene_.lights[1];
        break;
      case Key::O:
        light = &scene_.lights[2];
        break;
    }

    if (light) {
      light->enabled = !light->enabled;
    }
  }

  return true;
}
}  // namespace peanut