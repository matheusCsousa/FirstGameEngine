#include "Window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

namespace { unsigned int glfwOwners = 0; }

namespace Core {

Window::Window(const WindowSpecs& specs)
    : m_specs(specs) {}

Window::~Window() {
    destroy();
}

void Window::create() {
    if (m_window || m_ownsGlfw) throw std::logic_error("Window is already created");
    if (m_specs.width <= 0 || m_specs.height <= 0)
        throw std::invalid_argument("Window dimensions must be positive");
    if (glfwOwners == 0) {
        glfwSetErrorCallback([](int code, const char* description) {
            std::cerr << "GLFW error " << code << ": " << description << '\n';
        });
        if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");
    }
    ++glfwOwners;
    m_ownsGlfw = true;
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, m_specs.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_window = glfwCreateWindow(m_specs.width, m_specs.height, m_specs.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        destroy();
        throw std::runtime_error("Failed to create GLFW window with OpenGL 3.3");
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* w, int width, int height) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        win->m_specs.width = width;
        win->m_specs.height = height;
        if (win->m_eventCallback) {
            WindowResizeEvent event(width, height);
            win->m_eventCallback(event);
        }
    });

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* w) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (win->m_eventCallback) {
            WindowCloseEvent event;
            win->m_eventCallback(event);
        }
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        (void)scancode; (void)mods;
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (win->m_eventCallback) {
            if (action == GLFW_PRESS) {
                KeyPressedEvent event(key, 0);
                win->m_eventCallback(event);
            } else if (action == GLFW_RELEASE) {
                KeyReleasedEvent event(key);
                win->m_eventCallback(event);
            } else if (action == GLFW_REPEAT) {
                KeyPressedEvent event(key, 1);
                win->m_eventCallback(event);
            }
        }
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, int button, int action, int mods) {
        (void)mods;
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (win->m_eventCallback) {
            if (action == GLFW_PRESS) {
                MouseButtonPressedEvent event(button);
                win->m_eventCallback(event);
            } else if (action == GLFW_RELEASE) {
                MouseButtonReleasedEvent event(button);
                win->m_eventCallback(event);
            }
        }
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, double xpos, double ypos) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (win->m_eventCallback) {
            MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            win->m_eventCallback(event);
        }
    });

    glewExperimental = GL_TRUE;

    const auto glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        const std::string message = reinterpret_cast<const char*>(glewGetErrorString(glewStatus));
        destroy();
        throw std::runtime_error("Failed to initialize GLEW: " + message);
    }
    // GLEW probes extensions on core contexts; discard its startup error state.
    while (glGetError() != GL_NO_ERROR) {}
    glfwGetFramebufferSize(m_window, &m_specs.bufferWidth, &m_specs.bufferHeight);
    glfwSwapInterval(1);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, m_specs.bufferWidth, m_specs.bufferHeight);
}

void Window::destroy() noexcept {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_ownsGlfw) {
        m_ownsGlfw = false;
        if (--glfwOwners == 0) glfwTerminate();
    }
}

void Window::makeCurrent() noexcept {
    if (m_window) glfwMakeContextCurrent(m_window);
}

bool Window::hasFramebuffer() const {
    return m_specs.bufferWidth > 0 && m_specs.bufferHeight > 0;
}

void Window::update() {
    glfwSwapBuffers(m_window);
}

void Window::framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
) {
    glViewport(0, 0, width, height);

    Window* win = static_cast<Window*>(
        glfwGetWindowUserPointer(window)
    );

    win->m_specs.bufferWidth = width;
    win->m_specs.bufferHeight = height;
}

float Window::aspectRatio() const {
    return hasFramebuffer() ? static_cast<float>(m_specs.bufferWidth) / m_specs.bufferHeight : 1.0f;
}

void Window::swapBuffers() { glfwSwapBuffers(m_window); }

bool Window::shouldClose() { return !m_window || glfwWindowShouldClose(m_window); }

void Window::setShouldClose(bool value) { glfwSetWindowShouldClose(m_window, value); }

void Window::pollEvents() { glfwPollEvents(); }

void Window::requestClose() {
    glfwSetWindowShouldClose(m_window, true);
}
}
