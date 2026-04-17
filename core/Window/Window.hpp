#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>


namespace Core {

    struct WindowSpecs {
        int width = 800;
        int height = 600;
    };

    class Window {
    private:
        GLFWwindow* m_window = nullptr;
        WindowSpecs m_specs;

        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    public:
        Window(const WindowSpecs& specs);
        ~Window();

        GLFWwindow* getWindow() { return m_window; }

        void create();
        void destroy();
        void update();

        void swapBuffers() { glfwSwapBuffers(m_window); }
        bool shouldClose() { return glfwWindowShouldClose(m_window); }
        void setShouldClose(bool value) { glfwSetWindowShouldClose(m_window, value); }
        void pollEvents() { glfwPollEvents(); }
    };

}
