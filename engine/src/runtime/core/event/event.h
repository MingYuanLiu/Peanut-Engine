#pragma once

#include <string>

namespace peanut {

enum EventType {
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,
  KeyPressed,
  KeyReleased,
  KeyTyped,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled
};

#define EVENT_CLASS_TYPE(type)                                                \
  static EventType GetStaticType() { return EventType::type; }                \
  virtual EventType GetEventType() const override { return GetStaticType(); } \
  virtual const char* GetName() const override { return #type; }

#define BIND_EVENT_FN(fn)                                   \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

class Event {
 public:
  Event() : handled_(false) {}
  Event(const Event& event) = default;
  virtual ~Event() = default;
  virtual EventType GetEventType() const = 0;
  virtual const char* GetName() const = 0;
  virtual std::string ToString() const { return GetName(); }

  bool handled_;
};

class EventDispatcher {
 public:
  EventDispatcher(Event& event) : event_(event) {}

  template <typename T, typename F>
  bool Dispatch(const F& func) {
    if (event_.GetEventType() == T::GetStaticType()) {
      event_.handled_ |= func(static_cast<T&>(event_));
      return true;
    }

    return false;
  }

 private:
  Event& event_;
};
}  // namespace peanut