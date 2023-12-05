#include "runtime/functions/window/window_system.h"

#include "runtime/core/base/logger.h"
#include "runtime/core/event/key_event.h"
#include "runtime/core/event/mouse_event.h"
#include "runtime/core/event/window_event.h"

namespace peanut {
WindowSystem::WindowSystem(const WindowCreateInfo& create_info)
    : m_glf_window_(nullptr), intialized_(false) {
  PEANUT_LOG_INFO("Create window system ({0}, {1})", create_info.width,
                  create_info.height);
  Initialize(create_info);
}

WindowSystem::~WindowSystem() {}

void WindowSystem::OnUpdate() { glfwPollEvents(); }

void WindowSystem::Shutdown() {
  if (m_glf_window_) glfwDestroyWindow(m_glf_window_);

  if (intialized_) glfwTerminate();
}

void WindowSystem::SetVsync(bool enabled) {
  if (enabled)
    glfwSwapInterval(1);
  else
    glfwSwapInterval(0);

  window_data_.vsync = enabled;
}

void WindowSystem::SetMouseCapture(bool enabled) {
  window_data_.mouse_capture = enabled;
  glfwSetInputMode(m_glf_window_, GLFW_CURSOR,
                   enabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void WindowSystem::Initialize(const WindowCreateInfo& create_info) {
  window_data_.create_info = create_info;
  PEANUT_LOG_INFO("Intialize window system with size ({0}, {1}) title: {2}",
                  create_info.width, create_info.height, create_info.title);

  if (!intialized_) {
    if (!glfwInit()) {
      PEANUT_LOG_FATAL("Failed to initialize glf window");
      return;
    }

    intialized_ = true;
  }

  // create glf window
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  m_glf_window_ = glfwCreateWindow(create_info.width, create_info.height,
                                   create_info.title.c_str(), nullptr, nullptr);
  if (!m_glf_window_) {
    PEANUT_LOG_FATAL("Failed to create glf window");
    glfwTerminate();
    return;
  }

  // set user data
  glfwSetWindowUserPointer(m_glf_window_, &window_data_);
  SetVsync(true);

  // register window resize callback
  glfwSetWindowSizeCallback(
      m_glf_window_, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.create_info.width = width;
        data.create_info.height = height;

        // WindowResizeEvent
        WindowResizeEvent event(width, height);
        for (auto callback : data.event_callback) {
          callback(event);
        }
      });

  // register window close callback
  glfwSetWindowCloseCallback(m_glf_window_, [](GLFWwindow* window) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    // WindowCloseEvent
    WindowCloseEvent event;
    for (auto callback : data.event_callback) {
      callback(event);
    }
  });

  // register key char callback
  glfwSetKeyCallback(m_glf_window_, [](GLFWwindow* window, int key,
                                       int scancode, int action, int mods) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

    switch (action) {
      case GLFW_PRESS: {
        // key press event
        KeyPressedEvent event(key);
        for (auto callback : data.event_callback) {
          callback(event);
        }
      }
      case GLFW_RELEASE: {
        // key release event
        KeyReleasedEvent event(key);
        for (auto callback : data.event_callback) {
          callback(event);
        }
      }
      case GLFW_REPEAT: {
        // key repeat event
        KeyPressedEvent event(key, true);
        for (auto callback : data.event_callback) {
          callback(event);
        }
      }
    }
  });

  // register char callback
  glfwSetCharModsCallback(
      m_glf_window_, [](GLFWwindow* window, uint32_t code_point, int mods) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

        // key char event
        KeyTypedEvent event(code_point);
        for (auto callback : data->event_callback) {
          callback(event);
        }
      });

  // register mouse button callback
  glfwSetMouseButtonCallback(
      m_glf_window_, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
        switch (action) {
          case GLFW_PRESS: {
            // mouse button press
            MouseButtonPressedEvent event((MouseCode)button);
            for (auto callback : data->event_callback) {
              callback(event);
            }
          }
          case GLFW_RELEASE: {
            // mouse button release event
            MouseButtonReleasedEvent event((MouseCode)button);
            for (auto callback : data->event_callback) {
              callback(event);
            }
          }
        }
      });

  glfwSetScrollCallback(
      m_glf_window_, [](GLFWwindow* window, double x_offset, double y_offset) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
        // scroll event
        MouseScrolledEvent event(x_offset, y_offset);
        for (auto callback : data->event_callback) {
          callback(event);
        }
      });

  glfwSetCursorPosCallback(
      m_glf_window_, [](GLFWwindow* window, double x_pos, double y_pos) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
        // mouse position event
        MouseMovedEvent event(x_pos, y_pos);
        for (auto callback : data->event_callback) {
          callback(event);
        }
      });

  glfwSetInputMode(m_glf_window_, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

}  // namespace peanut