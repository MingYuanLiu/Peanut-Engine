#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "runtime/core/base/logger.h"
#include "runtime/core/event/event.h"
#include "runtime/functions/window/window.h"

namespace peanut {
/**
 * @brief The root window system implementing by glfw
 */
class WindowSystem : public Window {
 public:
  WindowSystem();
  virtual ~WindowSystem();

  virtual void OnUpdate() override;
  virtual uint32_t GetWidth() override {
    return window_data_.create_info.width;
  }
  virtual uint32_t GetHeight() override {
    return window_data_.create_info.height;
  }

  virtual void PushEventCallback(const EventCallbackFunc& callback) override {
    window_data_.event_callback.push_back(callback);
  }
  virtual void PopEventCallback() override {
    window_data_.event_callback.pop_back();
  }

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