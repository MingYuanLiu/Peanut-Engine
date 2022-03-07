#pragma once

#include "pepch.h"
#include "Peanut/Core.h"
#include "Peanut/Event/Event.h"

namespace Peanut_Engine
{
    struct WindowProps
    {
        std::string title_;
        uint16_t width_;
        uint16_t height_;

        WindowProps(const std::string& title = "Peanut Engine", 
                    uint16_t width = 1280, uint16_t height = 720)
                    : title_(title), width_(width), height_(height) {}
    };

    class PE_API Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {}

        // pure virtual functions
        virtual void Update() = 0;

        virtual uint16_t GetWidth() const = 0;
        virtual uint16_t GetHeight() const = 0;

        virtual void SetWindowEventCallback(const EventCallbackFn& callback_func) = 0;
        
        virtual void SetVSync(bool enable) = 0;
        virtual bool IsVSync() const = 0;

        static Window* Create(const WindowProps& = WindowProps());
    };
}