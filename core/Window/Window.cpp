#include "Window.hpp"

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

        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

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

    void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        (void)window;
        glViewport(0, 0, width, height);
    }
}
