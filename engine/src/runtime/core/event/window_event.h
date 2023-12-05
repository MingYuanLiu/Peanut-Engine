#pragma once

#include <sstream>
#include <string>

#include "runtime/core/event/event.h"

namespace peanut {
class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(uint16_t width, uint16_t height)
      : new_width_(width), new_height_(height) {}

  uint16_t GetWidth() const { return new_width_; }
  uint16_t GetHeight() const { return new_height_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "WindowResizeEvent: " << new_width_ << "," << new_height_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(WindowResize)

 private:
  uint16_t new_width_;
  uint16_t new_height_;
};

class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  std::string ToString() const override {
    std::stringstream ss;
    ss << "WindowCloseEvent ";
    return ss.str();
  }
  EVENT_CLASS_TYPE(WindowClose)
};
}  // namespace peanut