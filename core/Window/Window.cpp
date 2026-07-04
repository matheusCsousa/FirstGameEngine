#include "Window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Core {

Window::Window(const WindowSpecs& specs)
    : m_specs(specs) {}

Window::~Window() {
    destroy();
}

void Window::create() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_window = glfwCreateWindow(m_specs.width, m_specs.height, "OpenGL Window", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
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

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, m_specs.width, m_specs.height);
}

void Window::destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
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

float Window::aspectRatio() {
    return (float)m_specs.bufferWidth / (float)m_specs.bufferHeight;
}

void Window::swapBuffers() { glfwSwapBuffers(m_window); }

bool Window::shouldClose() { return glfwWindowShouldClose(m_window); }

void Window::setShouldClose(bool value) { glfwSetWindowShouldClose(m_window, value); }

void Window::pollEvents() { glfwPollEvents(); }

void Window::requestClose() {
    glfwSetWindowShouldClose(m_window, true);
}
}
