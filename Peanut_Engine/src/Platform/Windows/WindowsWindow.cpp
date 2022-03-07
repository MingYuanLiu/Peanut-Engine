#include "pepch.h"
#include "WindowsWindow.h"

namespace Peanut_Engine
{
    static bool s_GLFWInitialized = false;

    Window* Window::Create(const WindowProps& window_props)
    {
        return new WindowsWindow(window_props);
    }

    WindowsWindow::WindowsWindow(const WindowProps& window_props)
    {
        Init(window_props);
    }

    WindowsWindow::~WindowsWindow()
    {
        ShutDown();
    }

    void WindowsWindow::Init(const WindowProps& window_props)
    {
        data_.title_ = window_props.title_;
        data_.width_ = window_props.width_;
        data_.height_ = window_props.height_;

        PE_CORE_INFO("Create Window {0} <<< ({1},{2})", data_.title_, data_.width_, data_.height_);

        if (!s_GLFWInitialized) {
            int success = glfwInit();
            PE_CORE_ASSERT(x, "Could not intialize GLFW.");

            s_GLFWInitialized = true;
        }

        window_ = glfwCreateWindow((int)data_.width_, (int)data_.height_, data_.title_.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, &data_);
        SetVSync(true);
    }

    void WindowsWindow::ShutDown()
    {
        glfwDestroyWindow(window_);
    }

    void WindowsWindow::Update()
    {
        glfwPollEvents();
        glfwSwapBuffers(window_);
    }

    void WindowsWindow::SetVSync(bool enable)
    {
        if (enable) {
            glfwSwapInterval(1);
        }
        else {
            glfwSwapInterval(0);
        }

        data_.VSync_ = enable;
    }

    bool WindowsWindow::IsVSync() const
    {
        return data_.VSync_;
    }





}