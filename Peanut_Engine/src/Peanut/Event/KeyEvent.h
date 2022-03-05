#pragma once

#include "Event.h"
#include "Peanut/KeyboardCodes.h"

namespace Peanut_Engine
{

    class PE_API KeyEvent : public Event
    {
    public:
        inline KeyCode GetKeyCode() { return m_key_code; }

        CREATE_EVENT_CATEGORY(EventCategoryKeyboard | EventCategoryInput);
    protected:
        KeyEvent(int key_code) 
            : m_key_code(key_code) {}

        KeyCode m_key_code;
    };

    class PE_API KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int key_code, int repeat_count) 
            : KeyEvent(key_code), m_repeat_count(repeat_count) {}
        
        inline uint16_t GetRepeatCount() const { return m_repeat_count; }

        // used by debug 
        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressed: " << m_key_code << ", Repeat count: " << m_repeat_count;
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(KeyPressed);

    protected:
        uint16_t m_repeat_count;
    };

    class PE_API KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(KeyCode key_code)
            : KeyEvent(key_code) {}
        
        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyReleased: " << m_key_code;
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(KeyPressed);
    };

    class PE_API KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(KeyCode key_code)
            : KeyEvent(key_code) {}
        
        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyTyped: " << m_key_code;
            return ss.str();
        }

        CREATE_EVENT_CLASS_TYPE(KeyTyped);
    };

}