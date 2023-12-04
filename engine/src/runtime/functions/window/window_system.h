#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "runtime/core/base/logger.h"
#include "runtime/core/event/event.h"
#include "runtime/core/event/window.h"

namespace peanut {
/**
 * @brief The root window system implementing by glfw
 */
class WindowSystem : public Window {
 public:
  WindowSystem() : m_glf_window_(nullptr), intialized_(false) {}
  explicit WindowSystem(const WindowCreateInfo& create_info);
  virtual ~WindowSystem();

  virtual void OnUpdate() override;
  virtual uint32_t GetWidth() override { return create_info_.width; }
  virtual uint32_t GetHeight() override { return create_info_.height; }

  virtual void PushEventCallback(const EventCallbackFunc& callback) override;
  virtual const EventCallbackFunc& PopEventCallback() override;

  virtual void* GetNativeWindow() override { return m_glf_window_; };
  virtual void SetVsync(bool enabled) override;
  virtual bool IsVsync() override { return window_data_.vsync; }

  void SetMouseCapture(bool enabled);
  bool IsMouseCapture() { return window_data_.mouse_capture; }
  void Initialize(const WindowCreateInfo& create_info);
  void Shutdown();

 private:
  typedef struct {
    WindowCreateInfo create_info;
    std::vector<EventCallbackFunc> event_callback;
    bool vsync;
    bool mouse_capture;
  } WindowData;

  WindowData window_data_;
  GLFWwindow* m_glf_window_;
  bool intialized_;
};
}  // namespace peanut