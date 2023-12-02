#pragma once

#include "Event.h"

namespace Peanut_Engine
{
    class PE_API WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;
    
        CREATE_EVENT_CATEGORY(EventCategoryApplication);
        CREATE_EVENT_CLASS_TYPE(WindowClose);
    };

    class PE_API WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int width, int height)
            : m_width_(width), m_height_(height) {}

        std::string ToString() const override {
            std::stringstream ss;
            ss << "Windows reisze to: " << m_width_ << ", " << m_height_;
            return ss.str();
        }

        CREATE_EVENT_CATEGORY(EventCategoryApplication);
        CREATE_EVENT_CLASS_TYPE(WindowResize);
    private:
        int m_width_;
        int m_height_;
    };

    class PE_API AppTickEvent : public Event
    {
    public:
        AppTickEvent() = default;

        CREATE_EVENT_CATEGORY(EventCategoryApplication);
        CREATE_EVENT_CLASS_TYPE(AppTick);
    };

    class PE_API AppUpdateEvent : public Event
    {
    public:
        AppUpdateEvent() = default;

        CREATE_EVENT_CATEGORY(EventCategoryApplication);
        CREATE_EVENT_CLASS_TYPE(AppUpdate);
    };

    class PE_API AppRenderEvent : public Event
    {
    public:
        AppRenderEvent() = default;

        CREATE_EVENT_CATEGORY(EventCategoryApplication);
        CREATE_EVENT_CLASS_TYPE(AppRender);
    };
}