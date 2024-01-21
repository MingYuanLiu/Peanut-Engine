#pragma once

#include <sstream>
#include <string>

#include "runtime/core/base/keycode.h"
#include "runtime/core/event/event.h"

namespace peanut {

class KeyEvent : public Event {
 public:
  KeyEvent(const KeyCode keycode, bool pressed, bool repeat) : keycode_(keycode), pressed_(pressed), repeat_(repeat) {}
  KeyCode GetKeyCode() const { return keycode_; }
  bool IsPressed() { return pressed_; }
  bool IsRepeat() { return repeat_; }

  std::string ToString() const override {
      std::stringstream ss;
      ss << "KeyBoardEvent: " << keycode_ << "(pressed=" << pressed_ <<")," << "(repeat=" << repeat_ << ")";
      return ss.str();
  }

  EVENT_CLASS_TYPE(KeyBoard)

 protected:
  KeyCode keycode_;
  bool pressed_;
  bool repeat_;
};

class KeyTypedEvent : public KeyEvent {
 public:
  KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode, false, false) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyTypedEvent: " << keycode_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyTyped)
};
}  // namespace peanut