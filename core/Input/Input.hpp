#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Core {
    class Input {
    public:
        enum KeyState {
            KEY_RELEASED,
            KEY_PRESSED,
            KEY_HELD
        };
        static void initialize(GLFWwindow* window);

        static KeyState getKeyState(int key);
        static void update();
        static void handleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void handleMouseCallback(GLFWwindow* window, double xpos, double ypos);

        static void setMouseMode(GLFWwindow* window, bool disabled);
        static int getMouseMode(GLFWwindow* window) { return glfwGetInputMode(window, GLFW_CURSOR); };

        static glm::dvec2 getMouseChange();

        static GLfloat getDeltaTime();

    private:
        static KeyState keys[1024];
        static double lastX;
        static double lastY;
        static double xChange;
        static double yChange;
        static GLfloat deltaTime;
        static GLfloat lastTime;
        static bool mouseFirstMoved;

    };
}
