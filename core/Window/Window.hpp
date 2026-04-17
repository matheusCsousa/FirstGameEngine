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
        GLFWwindow* window = nullptr;
        WindowSpecs& m_specs;

    public:
        Window(WindowSpecs& specs);
        ~Window();

        void create();
        void destroy();
    };

}
