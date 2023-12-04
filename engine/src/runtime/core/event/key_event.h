#pragma once

#include <sstream>
#include <string>

#include "runtime/core/base/keycode.h"
#include "runtime/core/event/event.h"

namespace peanut {

class KeyEvent : public Event {
 public:
  KeyCode GetKeyCode() const { return keycode_; }

 protected:
  KeyEvent(const KeyCode keycode) : keycode_(keycode) {}
  KeyCode keycode_;
};

class KeyPressedEvent : public KeyEvent {
 public:
  KeyPressedEvent(const KeyCode keycode, bool is_repeat = false)
      : KeyEvent(keycode), is_repeat_(is_repeat) {}

  bool IsRepeat() const { return is_repeat_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyPressedEvent: " << keycode_ << "(repeat=" << is_repeat_ << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)
 private:
  bool is_repeat_;
};

class KeyReleasedEvent : public KeyEvent {
 public:
  KeyReleasedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyReleaseEvent: " << keycode_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent {
 public:
  KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyTypedEvent: " << keycode_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyTyped)
};
}  // namespace peanut