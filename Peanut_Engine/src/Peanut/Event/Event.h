#pragma once

#include "Peanut/Core.h"

namespace Peanut_Engine
{
    // The current implementation mode is to process the event immediately after receiving it,
    // and other events need to wait, which is the blocking mode.
    // In the future, we can consider using a buffer queue for implementation.
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,  // Window Events
        AppTick, AppUpdate, AppRender,                                         // Application Events
        KeyPressed, KeyReleased, KeyTyped,                                     // Keyboard Events
        MouseButtonPressed, MouseButtonReleased, 
        MouseMoved, MouseScrolled                                              // Mouse Button Events
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication        = BIT(0),
        EventCategoryInput              = BIT(1),
        EventCategoryKeyboard           = BIT(2),
        EventCategoryMouse              = BIT(3),
        EventCategoryMouseButton        = BIT(4),
    };

#define CREATE_EVENT_CLASS_TYPE(type)      static EventType GetStaticType() { return EventType::##type; } \
                                            virtual EventType GetEventType() const override { return GetStaticType(); }    \
                                            virtual const char* GetName() const override { return #type; }

#define CREATE_EVENT_CATEGORY(category)    virtual int GetEventCategoryFlags() const override { return category; }

    class PE_API Event
    {
    public:
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetEventCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        inline bool IsInCategory(EventCategory& category) {
            return GetEventCategoryFlags() & category;
        }

        bool m_handled = false;
    private:
        friend class EventDispatcher;
    };

    class EventDispatcher
    {
    public:
        EventDispatcher(Event& event)
            : m_event(event) {}

        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_event.GetEventType() == T::GetStaticType()) {
                m_event.m_handled |= func(static_cast<T&>(m_event));

                return true;
            }
            else {
                return false;
            }
        }

    private:
        Event& m_event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {

        return os << e.ToString();
    }
}