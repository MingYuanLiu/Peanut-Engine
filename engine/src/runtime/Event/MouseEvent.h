#pragma once

#include "Event.h"
#include "Peanut/MouseCode.h"

namespace Peanut_Engine
{
    class PE_API MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(const float mouseX, const float mouse_y)
            : m_mouseX_(mouseX), m_mouseY_(mouse_y) {}
        
        inline float GetMouseX() const { return m_mouseX_; }
        inline float GetMouseY() const { return m_mouseY_; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "Mouse move: (" << m_mouseX_  << ", " << m_mouseY_ << ")";
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(MouseMoved);
        CREATE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput);
    
    protected:
        float m_mouseX_, m_mouseY_;
    };

    class PE_API MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(const float offsetX, const float offsetY)
            : m_offsetX_(offsetX), m_offsetY_(offsetY) {}

        inline float GetOffsetX() { return m_offsetX_; }
        inline float GetOffsetY() { return m_offsetY_; }
 
        std::string ToString() const override {
            std::stringstream ss;
            ss << "Mouse scroll, offset is (" << m_offsetX_  << ", " << m_offsetY_ << ")";
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(MouseScrolled);
        CREATE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput);

    protected:
        float m_offsetX_, m_offsetY_;
    };

    class PE_API MouseButtonEvent : public Event
    {
    public:
        inline MouseCode GetMouseCode() { return m_mouse_code_; }

        CREATE_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton);

    protected:
        MouseButtonEvent(MouseCode mouse_code)
            : m_mouse_code_(mouse_code) {}

        MouseCode m_mouse_code_;
    };

    class PE_API MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(MouseCode mouse_code, uint16_t repeat_count)
            : MouseButtonEvent(mouse_code), m_repeat_count_(repeat_count) {}
        
        std::string ToString() const override {
            std::stringstream ss;
            ss << "Mouse button pressed: " << m_mouse_code_  << ", repeat conut: " << m_repeat_count_;
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(MouseButtonPressed);
    protected:
        uint16_t m_repeat_count_;
    };

    class PE_API MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(MouseCode mouse_code)
            : MouseButtonEvent(mouse_code) {}
        
        std::string ToString() const override {
            std::stringstream ss;
            ss << "Mouse button released: " << m_mouse_code_;
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(MouseButtonReleased);
    };

}