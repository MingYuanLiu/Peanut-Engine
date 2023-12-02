#pragma once

#include "Peanut/Window.h"
#include "Peanut/Log.h"

#include <GLFW/glfw3.h>

namespace Peanut_Engine
{
    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowProps& window_props);
        virtual ~WindowsWindow();

        virtual void Update() override;

        virtual uint16_t GetWidth() const override { return data_.width_; }
        virtual uint16_t GetHeight() const override { return data_.height_; }

        virtual void SetWindowEventCallback(const EventCallbackFn& callback_func) override { data_.callback_func_ = callback_func; }
        virtual void SetVSync(bool enable) override;
        virtual bool IsVSync() const override;

    private:
        virtual void Init(const WindowProps& window_props);
        virtual void ShutDown();
    
    private:
        GLFWwindow* window_;

        struct WindowData
        {
            std::string title_;
            uint16_t width_;
            uint16_t height_;
            bool VSync_;

            EventCallbackFn callback_func_;
        };

        WindowData data_;
    };
}