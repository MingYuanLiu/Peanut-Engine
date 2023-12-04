#include "pepch.h"
#include "WindowsWindow.h"

#include "Peanut/Event/ApplicationEvent.h"
#include "Peanut/Event/KeyEvent.h"
#include "Peanut/Event/MouseEvent.h"

namespace Peanut_Engine
{
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallBack(int error, const char* description)
    {
        PE_CORE_ERROR("GLFW Error ({0}) : {1}", error, description);
    }

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

            glfwSetErrorCallback(GLFWErrorCallBack);
            s_GLFWInitialized = true;
        }

        window_ = glfwCreateWindow((int)data_.width_, (int)data_.height_, data_.title_.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, &data_);
        SetVSync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.width_ = width;
            data.height_ = height;

            WindowResizeEvent event(width, height);
            data.callback_func_(event);

        });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.callback_func_(event);
        });

        glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event((KeyCode)key, 0);
                    data.callback_func_(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event((KeyCode)key);
                    data.callback_func_(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event((KeyCode)key, 1);
                    data.callback_func_(event);
                    break;
                }
                default:
                    break;
            }
        });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button, 0);
                    data.callback_func_(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.callback_func_(event);
                    break;
                }
                default:
                    break;
                }
        });

        glfwSetScrollCallback(window_, [](GLFWwindow* window, double offset_x, double offset_y)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)offset_x, (float)offset_y);
            data.callback_func_(event);
        });

        glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double pos_x, double pos_y)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)pos_x, (float)pos_y);
            data.callback_func_(event);
        });
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