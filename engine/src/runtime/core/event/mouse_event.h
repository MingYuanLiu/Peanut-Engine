#pragma once

#include "runtime/core/base/mouse_code.h"
#include "runtime/core/event/event.h"

namespace peanut {

class MouseButtonEvent : public Event {
 public:
  MouseCode GetMouseButton() const { return mouse_button_; }

 protected:
  MouseButtonEvent(const MouseCode button) : mouse_button_(button) {}
  MouseCode mouse_button_;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
 public:
  MouseButtonPressedEvent(const MouseCode button) : MouseButtonEvent(button) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseButtonPressedEvent: " << mouse_button_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
 public:
  MouseButtonReleasedEvent(const MouseCode button) : MouseButtonEvent(button) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseButtonReleaseEvent: " << mouse_button_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonReleased)
};

class MouseMovedEvent : public Event {
 public:
  MouseButtonEvent(double x_pos, double y_pos) : x_pos_(x_pos), y_pos_(y_pos) {}

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