#pragma once

#include <sstream>
#include <string>

#include "runtime/core/base/mouse_code.h"
#include "runtime/core/event/event.h"

namespace peanut {

class MouseButtonEvent : public Event {
 public:
  MouseButtonEvent(const MouseCode button, bool pressed) : mouse_button_(button), pressed_(pressed) {}
  MouseCode GetMouseButton() const { return mouse_button_; }
  bool IsPressed() { return pressed_; }

  std::string ToString() const override {
      std::stringstream ss;
      ss << "MouseButtonEvent: " << mouse_button_;
      if (pressed_)
          ss << ", pressed";
      else
          ss << ", release";
      return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButton)
 protected:
  MouseCode mouse_button_;
  bool pressed_;
};

class MouseMovedEvent : public Event {
 public:
  MouseMovedEvent(double x_pos, double y_pos) : x_pos_(x_pos), y_pos_(y_pos) {}

  double GetX() { return x_pos_; }
  double GetY() { return y_pos_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseMovedEvent: "
       << "(" << x_pos_ << "," << y_pos_ << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseMoved)

 private:
  double x_pos_;
  double y_pos_;
};

class MouseScrolledEvent : public Event {
 public:
  MouseScrolledEvent(double x_offset, double y_offset)
      : x_offset_(x_offset), y_offset_(y_offset) {}

  double GetXOffset() { return x_offset_; }
  double GetYOffset() { return y_offset_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseScrolledEvent: offset -> "
       << "(" << x_offset_ << "," << y_offset_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseScrolled)

 private:
  double x_offset_;
  double y_offset_;
};
}  // namespace peanut