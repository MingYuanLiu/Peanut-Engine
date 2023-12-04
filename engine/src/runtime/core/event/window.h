#pragma once

#include <functional>
#include <string>

#include "runtime/core/event/event.h"
namespace peanut {

typedef struct {
  uint32_t width;
  uint32_t height;
  std::string title;

  WindowCreateInfo(uint32_t width = 1280, uint32_t height = 720,
                   const std::string& title = "Peanut Engine")
      : width(width), height(height), title(title) {}
} WindowCreateInfo;

/**
 * @brief Abstract window class
 *
 */
class Window {
 public:
  using EventCallbackFunc = std::function<void(Event&)>;

  virtual ~Window() = default;

  virtual void OnUpdate() = 0;
  virtual uint32_t GetWidth() = 0;
  virtual uint32_t GetHeight() = 0;

  virtual void PushEventCallback(const EventCallbackFunc& callback) = 0;
  virtual const EventCallbackFunc& PopEventCallback() = 0;

  virtual void SetVsync(bool enabled) = 0;
  virtual bool IsVsync() = 0;
  virtual void* GetNativeWindow() = 0;
};
}  // namespace peanut